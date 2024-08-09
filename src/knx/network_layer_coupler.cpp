#include "network_layer_coupler.h"
#include "data_link_layer.h"
#include "device_object.h"
#include "router_object.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"

NetworkLayerCoupler::NetworkLayerCoupler(DeviceObject &deviceObj,
                                         TransportLayer& layer) :
    NetworkLayer(deviceObj, layer),
    _netLayerEntities { {*this, kPrimaryIfIndex}, {*this, kSecondaryIfIndex} }
{
    _currentAddress = deviceObj.individualAddress();
    evaluateCouplerType();
}

NetworkLayerEntity& NetworkLayerCoupler::getPrimaryInterface()
{
    return _netLayerEntities[0];
}

NetworkLayerEntity& NetworkLayerCoupler::getSecondaryInterface()
{
    return _netLayerEntities[1];
}

void NetworkLayerCoupler::rtObj(RouterObject& rtObj)
{
    _rtObjPrimary = &rtObj;
    _rtObjSecondary = nullptr;
}

void NetworkLayerCoupler::rtObjPrimary(RouterObject& rtObjPrimary)
{
    _rtObjPrimary = &rtObjPrimary;
}

void NetworkLayerCoupler::rtObjSecondary(RouterObject& rtObjSecondary)
{
    _rtObjSecondary = &rtObjSecondary;
}

void NetworkLayerCoupler::evaluateCouplerType()
{
    // Check coupler mode
    if ((_deviceObj.individualAddress() & 0x00FF) == 0x00)
    {
        // Device is a router
        // Check if line coupler or backbone coupler
        if ((_deviceObj.individualAddress() & 0x0F00) == 0x0)
        {
            // Device is a backbone coupler -> individual address: x.0.0
            _couplerType = BackboneCoupler;
        }
        else
        {
            // Device is a line coupler -> individual address: x.y.0
            _couplerType = LineCoupler;
        }
    }
    else
    {
        // Device is not a router, check if TP1 bridge or TP1 repeater
/*
      if (PID_L2_COUPLER_TYPE.BIT0 == 0)
      {
        //then Device is TP1 Bridge
        couplerType = TP1Bridge;
      }
      else
      {
          // Device is TP1 Repeater
          couplerType = TP1Repeater;
      }
*/
    }
}

bool NetworkLayerCoupler::isGroupAddressInFilterTable(uint16_t groupAddress)
{
    if (_rtObjSecondary == nullptr)
        return (_rtObjPrimary!= nullptr) ? _rtObjPrimary->isGroupAddressInFilterTable(groupAddress) : false;
    else
    {
        return _rtObjSecondary->isGroupAddressInFilterTable(groupAddress);
    }
}

bool NetworkLayerCoupler::isRoutedGroupAddress(uint16_t groupAddress, uint8_t sourceInterfaceIndex)
{
    uint8_t lcconfig = LCCONFIG::PHYS_FRAME_ROUT | LCCONFIG::PHYS_REPEAT | LCCONFIG::BROADCAST_REPEAT | LCCONFIG::GROUP_IACK_ROUT | LCCONFIG::PHYS_IACK_NORMAL; // default value from spec. in case prop is not availible.
    uint8_t lcgrpconfig = LCGRPCONFIG::GROUP_6FFFROUTE | LCGRPCONFIG::GROUP_7000UNLOCK | LCGRPCONFIG::GROUP_REPEAT; // default value from spec. in case prop is not availible.
    Property* prop_lcgrpconfig;
    Property* prop_lcconfig;

    if(sourceInterfaceIndex == kPrimaryIfIndex) // direction Prim -> Sec ( e.g. IP -> TP)
    {
        prop_lcgrpconfig = _rtObjPrimary->property(PID_MAIN_LCGRPCONFIG);
        prop_lcconfig = _rtObjPrimary->property(PID_MAIN_LCCONFIG);
    }
    else // direction Sec -> Prim ( e.g. TP -> IP)
    {
        prop_lcgrpconfig = _rtObjPrimary->property(PID_SUB_LCGRPCONFIG);
        prop_lcconfig = _rtObjPrimary->property(PID_SUB_LCCONFIG);
    }
    if(prop_lcgrpconfig)
        prop_lcgrpconfig->read(lcgrpconfig);

    if(prop_lcconfig)
        prop_lcconfig->read(lcconfig);


    if(groupAddress < 0x7000) // Main group 0-13
    {
        // PID_SUB_LCGRPCONFIG Bit 0-1
        switch(lcgrpconfig & LCGRPCONFIG::GROUP_6FFF)
        {
            case LCGRPCONFIG::GROUP_6FFFLOCK:
                //printHex("1drop frame to 0x", (uint8_t*)destination, 2);
                return false;//drop
            break;
            case LCGRPCONFIG::GROUP_6FFFROUTE:
                if(isGroupAddressInFilterTable(groupAddress))
                    ;//send
                else
                {
                    //printHex("2drop frame to 0x", (uint8_t*)destination, 2);
                    return false;//drop
                }
            break;
            default: // LCGRPCONFIG::GROUP_6FFFUNLOCK
                ;//send
        }
    }
    else    // Main group 14-31
    {
        // PID_SUB_LCGRPCONFIG Bit 2-3 LCGRPCONFIG::GROUP_7000
        switch(lcgrpconfig & LCGRPCONFIG::GROUP_7000)
        {
            case LCGRPCONFIG::GROUP_7000LOCK:
                //printHex("3drop frame to 0x", (uint8_t*)destination, 2);
                return false;//drop
            break;
            case LCGRPCONFIG::GROUP_7000ROUTE:
                if(isGroupAddressInFilterTable(groupAddress))
                    ;//send
                else
                {
                    //printHex("4drop frame to 0x", (uint8_t*)destination, 2);
                    return false;//drop
                }
            break;
            default: // LCGRPCONFIG::GROUP_7000UNLOCK
                ;//send
        }
    }

    return true;
}

bool NetworkLayerCoupler::isRoutedIndividualAddress(uint16_t individualAddress, uint8_t srcIfIndex)
{
    // TODO: improve: we have to be notified about anything that might affect routing decision
    // Ugly: we could ALWAYS evaluate coupler type for every received frame
    if (_currentAddress != _deviceObj.individualAddress())
    {
        evaluateCouplerType();
    }

    // See KNX spec.: Network Layer (03/03/03) and AN161 (Coupler model 2.0)
    /*
        * C  hop count value contained in the N-protocol header
        * D  low order octet of the Destination Address, i.e. Device Address part
        * G  Group Address
        * SD low nibble of high order octet plus low order octet, i.e. Line Address + Device Address
        * Z  high nibble of high order octet of the Destination Address, i.e. Area Address
        * ZS high order octet of the Destination Address, i.e. hierarchy information part: Area Address + Line Address
    */
    uint16_t ownSNA = _deviceObj.individualAddress() & 0xFF00; // Own subnetwork address (area + line)
    uint16_t ownAA = _deviceObj.individualAddress() & 0xF000;  // Own area address
    uint16_t ZS = individualAddress & 0xFF00;                        // destination subnetwork address (area + line)
    uint16_t Z = individualAddress & 0xF000;                         // destination area address


    if (_couplerType == LineCoupler)
    {
        // Main line to sub line routing
        if (srcIfIndex == kPrimaryIfIndex)
        {
            if (ZS != ownSNA)
            {
                // IGNORE_TOTALLY
                return false;
            }
            return true;
        }
        else if (srcIfIndex == kSecondaryIfIndex) // Sub line to main line routing
        {
            if (ZS != ownSNA)
            {
                // ROUTE_XXX
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            //not from primiary not from sec if, should not happen
            return false;
        }
    }
    else if (_couplerType == BackboneCoupler)
    {
        // Backbone line to main line routing
        if (srcIfIndex == kPrimaryIfIndex)
        {
            if (Z != ownAA)
            {
                return false;
            }

            return true;
        }
        else  if (srcIfIndex == kSecondaryIfIndex)         // Main line to backbone line routing
        {
            if (Z != ownAA)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            //not from primiary not from sec if, should not happen
            return false;
        }
    }
    else
    {
        //unknown coupler type, should not happen
        return false;
    }
}

void NetworkLayerCoupler::sendMsgHopCount(AckType ack, AddressType addrType, uint16_t destination, NPDU& npdu, Priority priority,
                                          SystemBroadcast broadcastType, uint8_t sourceInterfaceIndex, uint16_t source)
{
    uint8_t interfaceIndex = (sourceInterfaceIndex == kSecondaryIfIndex) ? kPrimaryIfIndex : kSecondaryIfIndex;

    uint8_t lcconfig = LCCONFIG::PHYS_FRAME_ROUT | LCCONFIG::PHYS_REPEAT | LCCONFIG::BROADCAST_REPEAT | LCCONFIG::GROUP_IACK_ROUT | LCCONFIG::PHYS_IACK_NORMAL; // default value from spec. in case prop is not availible.
    uint8_t lcgrpconfig = LCGRPCONFIG::GROUP_6FFFROUTE | LCGRPCONFIG::GROUP_7000UNLOCK | LCGRPCONFIG::GROUP_REPEAT; // default value from spec. in case prop is not availible.
    Property* prop_lcgrpconfig;
    Property* prop_lcconfig;

    if(sourceInterfaceIndex == kPrimaryIfIndex) // direction Prim -> Sec ( e.g. IP -> TP)
    {
        prop_lcgrpconfig = _rtObjPrimary->property(PID_MAIN_LCGRPCONFIG);
        prop_lcconfig = _rtObjPrimary->property(PID_MAIN_LCCONFIG);
    }
    else // direction Sec -> Prim ( e.g. TP -> IP)
    {
        prop_lcgrpconfig = _rtObjPrimary->property(PID_SUB_LCGRPCONFIG);
        prop_lcconfig = _rtObjPrimary->property(PID_SUB_LCCONFIG);
    }
    if(prop_lcgrpconfig)
        prop_lcgrpconfig->read(lcgrpconfig);

    if(prop_lcconfig)
        prop_lcconfig->read(lcconfig);
    
    
    if(addrType == AddressType::GroupAddress && destination != 0) // destination == 0 means broadcast and must not be filtered with the GroupAddresses
    {
        if(!isRoutedGroupAddress(destination, sourceInterfaceIndex))
            return; // drop;
    }


    // If we have a frame from open medium on secondary side (e.g. RF) to primary side, then shall use the hop count of the primary router object
    if ((_rtObjPrimary != nullptr) && (_rtObjSecondary != nullptr) && (sourceInterfaceIndex == kSecondaryIfIndex))
    {
        DptMedium mediumType = getSecondaryInterface().mediumType();
        if (mediumType == DptMedium::KNX_RF) // Probably also KNX_PL110, but this is not specified, PL110 is also an open medium
        {
            uint16_t hopCount = 0;
            if (_rtObjPrimary->property(PID_HOP_COUNT)->read(hopCount) == 1)
            {
                npdu.hopCount(hopCount);
            }
        }
    }
    else // Normal hopCount between main and sub line and vice versa
    {
        if (npdu.hopCount() == 0)
        {
            // IGNORE_ACKED
            return;
        }
        if (npdu.hopCount() < 7)
        {
            // ROUTE_DECREMENTED
            npdu.hopCount(npdu.hopCount() - 1);
        }
        else if (npdu.hopCount() == 7)
        {
            // ROUTE_UNMODIFIED
        }
    }

    // Use other interface
#ifdef KNX_LOG_COUPLER
    if (sourceInterfaceIndex == 0)
        print("Routing from P->S: ");
    else
        print("Routing from S->P: ");
    print(source, HEX); print(" -> "); print(destination, HEX);
    print(" - ");
    npdu.frame().apdu().printPDU();
#endif

    //evaluiate PHYS_REPEAT, BROADCAST_REPEAT and GROUP_REPEAT
    bool doNotRepeat = false;
    if((addrType == AddressType::GroupAddress && !(lcgrpconfig & LCGRPCONFIG::GROUP_REPEAT)) ||
       (addrType == AddressType::IndividualAddress && !(lcconfig & LCCONFIG::PHYS_REPEAT)) ||
       (addrType == AddressType::GroupAddress && destination == 0 && !(lcconfig & LCCONFIG::BROADCAST_REPEAT)))
        doNotRepeat = true;
    
    _netLayerEntities[interfaceIndex].sendDataRequest(npdu, ack, destination, source, priority, addrType, broadcastType, doNotRepeat);
}

// TODO: for later: improve by putting routing algorithms in its own class/functions and only instantiate required algorithm (line vs. coupler)
// TODO: we could also do the sanity checks here, i.e. check if sourceAddress is really coming in from correct srcIfIdx, etc. (see PID_COUPL_SERV_CONTROL: EN_SNA_INCONSISTENCY_CHECK)
void NetworkLayerCoupler::routeDataIndividual(AckType ack, uint16_t destination, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIndex)
{
    //print("NetworkLayerCoupler::routeDataIndividual dest 0x");
    //print(destination, HEX);
    //print(" own addr 0x");
    //println(_deviceObj.individualAddress(), HEX);

    if(destination == _deviceObj.individualAddress())
    {
        // FORWARD_LOCALLY
        //println("NetworkLayerCoupler::routeDataIndividual locally");
        HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
        _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
        return;
    }

    // Local to main or sub line
    if (srcIfIndex == kLocalIfIndex)
    {
        uint16_t netaddr;
        uint16_t Z;
        if(_couplerType == CouplerType::BackboneCoupler)
        {
            netaddr = _deviceObj.individualAddress() & 0xF000;
            Z = destination & 0xF000;
        }
        else if(_couplerType == CouplerType::LineCoupler)
        {
            netaddr = _deviceObj.individualAddress() & 0xFF00;
            Z = destination & 0xFF00;
        }
        else
        {
            //unknown coupler type, should not happen
            return ;
        }
        

        // if destination is not within our scope then send via primary interface, else via secondary interface
        uint8_t destIfidx = (Z != netaddr) ? kPrimaryIfIndex : kSecondaryIfIndex;
#ifdef KNX_TUNNELING
        if(destIfidx == kPrimaryIfIndex)
            if(isTunnelAddress(destination))
                destIfidx = kSecondaryIfIndex;
#endif
        //print("NetworkLayerCoupler::routeDataIndividual local to s or p: ");
        //println(destIfidx);
        _netLayerEntities[destIfidx].sendDataRequest(npdu, ack, destination, source, priority, AddressType::IndividualAddress, Broadcast);
        return;
    }

    uint8_t lcconfig = LCCONFIG::PHYS_FRAME_ROUT | LCCONFIG::PHYS_REPEAT | LCCONFIG::BROADCAST_REPEAT | LCCONFIG::GROUP_IACK_ROUT | LCCONFIG::PHYS_IACK_NORMAL; // default value from spec. in case prop is not availible.
    Property* prop_lcconfig;
    if(srcIfIndex == kPrimaryIfIndex) // direction Prim -> Sec ( e.g. IP -> TP)
        prop_lcconfig = _rtObjPrimary->property(PID_MAIN_LCCONFIG);
    else // direction Sec -> Prim ( e.g. TP -> IP)
        prop_lcconfig = _rtObjPrimary->property(PID_SUB_LCCONFIG);
    if(prop_lcconfig)
        prop_lcconfig->read(lcconfig);

    if((lcconfig & LCCONFIG::PHYS_FRAME) == LCCONFIG::PHYS_FRAME_LOCK)
    {
        // IGNORE_TOTALLY
        //println("NetworkLayerCoupler::routeDataIndividual locked");
        return;
    }
    else if((lcconfig & LCCONFIG::PHYS_FRAME) == LCCONFIG::PHYS_FRAME_UNLOCK)
    {
        // ROUTE_XXX
        //println("NetworkLayerCoupler::routeDataIndividual unlocked");
        sendMsgHopCount(ack, AddressType::IndividualAddress, destination, npdu, priority, Broadcast, srcIfIndex, source);
        return;
    }
    else // LCCONFIG::PHYS_FRAME_ROUTE or 0
    {
        if(isRoutedIndividualAddress(destination, srcIfIndex))
        {
            //println("NetworkLayerCoupler::routeDataIndividual routed");
            sendMsgHopCount(ack, AddressType::IndividualAddress, destination, npdu, priority, Broadcast, srcIfIndex, source); // ROUTE_XXX
        }
        else
        {
            //println("NetworkLayerCoupler::routeDataIndividual not routed");
            ; // IGNORE_TOTALLY
        }
    }
}

void NetworkLayerCoupler::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    // routing for individual addresses
    if (addrType == IndividualAddress)
    {
        //printHex("NetworkLayerCoupler::dataIndication to IA ", (uint8_t*)&destination, 2);
        //npdu.frame().valid();
        routeDataIndividual(ack, destination, npdu, priority, source, srcIfIdx);
        return;
    }
    //printHex("NetworkLayerCoupler::dataIndication to GA ", (uint8_t*)&destination, 2);
    // routing for group addresses
    // TODO: check new AN189
    // "AN189 only makes that group messages with hop count 7 cannot bypass the Filter Table unfiltered,
    // what made the Security Proxy(AN192) useless; now, hc 7 Telegrams are filtered as any other and the value is decremented.

    // ROUTE_XXX
    sendMsgHopCount(ack, addrType, destination, npdu, priority, Broadcast, srcIfIdx, source);
    return;
}

void NetworkLayerCoupler::dataConfirm(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
    //println("NetworkLayerCoupler::dataConfirm");
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.individualAddress())
    {
        if (addrType == IndividualAddress)
        {
            _transportLayer.dataIndividualConfirm(ack, destination, hopType, priority, npdu.tpdu(), status);
            return;
        }
        // else: we do not have any local group communication, so do not handle this
    }

    // Do not process the frame any further if it was a routed frame sent from network layer
}

void NetworkLayerCoupler::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    // Send it to our local stack first
    {
        HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
        DptMedium mediumType = _netLayerEntities[srcIfIdx].mediumType();

        // for closed media like TP1 and IP
        if ( ((mediumType == DptMedium::KNX_TP1) || (mediumType == DptMedium::KNX_IP)) &&
              isApciSystemBroadcast(npdu.tpdu().apdu()))
        {
            npdu.frame().systemBroadcast(SysBroadcast);
            _transportLayer.dataSystemBroadcastIndication(hopType, priority, source, npdu.tpdu());
            return;
        }

        _transportLayer.dataBroadcastIndication(hopType, priority, source, npdu.tpdu());
    }

    uint8_t lcconfig = LCCONFIG::PHYS_FRAME_ROUT | LCCONFIG::PHYS_REPEAT | LCCONFIG::BROADCAST_REPEAT | LCCONFIG::GROUP_IACK_ROUT | LCCONFIG::PHYS_IACK_NORMAL; // default value from spec. in case prop is not availible.
    Property* prop_lcconfig;
    if(srcIfIdx == kPrimaryIfIndex) // direction Prim -> Sec ( e.g. IP -> TP)
        prop_lcconfig = _rtObjPrimary->property(PID_MAIN_LCCONFIG);
    else // direction Sec -> Prim ( e.g. TP -> IP)
        prop_lcconfig = _rtObjPrimary->property(PID_SUB_LCCONFIG);
    if(prop_lcconfig)
        prop_lcconfig->read(lcconfig);

    // Route to other interface
    if(!(lcconfig & LCCONFIG::BROADCAST_LOCK))
        sendMsgHopCount(ack, GroupAddress, 0, npdu, priority, Broadcast, srcIfIdx, source);
}

void NetworkLayerCoupler::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.individualAddress())
    {
         _transportLayer.dataBroadcastConfirm(ack, hopType, priority, npdu.tpdu(), status);
    }
    // Do not process the frame any further
}

void NetworkLayerCoupler::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    // Send it to our local stack first
    {
        HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
        _transportLayer.dataSystemBroadcastIndication(hopType, priority, source, npdu.tpdu());
    }
    
    uint8_t lcconfig = LCCONFIG::PHYS_FRAME_ROUT | LCCONFIG::PHYS_REPEAT | LCCONFIG::BROADCAST_REPEAT | LCCONFIG::GROUP_IACK_ROUT | LCCONFIG::PHYS_IACK_NORMAL; // default value from spec. in case prop is not availible.
    Property* prop_lcconfig;
    if(srcIfIdx == kPrimaryIfIndex) // direction Prim -> Sec ( e.g. IP -> TP)
        prop_lcconfig = _rtObjPrimary->property(PID_MAIN_LCCONFIG);
    else // direction Sec -> Prim ( e.g. TP -> IP)
        prop_lcconfig = _rtObjPrimary->property(PID_SUB_LCCONFIG);
    if(prop_lcconfig)
        prop_lcconfig->read(lcconfig);

    // Route to other interface
    if(!(lcconfig & LCCONFIG::BROADCAST_LOCK))
        sendMsgHopCount(ack, GroupAddress, 0, npdu, priority, SysBroadcast, srcIfIdx, source);
}

void NetworkLayerCoupler::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.individualAddress())
    {
        HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
        _transportLayer.dataSystemBroadcastConfirm(ack, hopType, npdu.tpdu(), priority, status);
    }
    // Do not process the frame any further
}

void NetworkLayerCoupler::dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    //if (tpdu.apdu().length() > 0)
    //{
    //    print.print("-> NL  ");
    //    tpdu.apdu().printPDU();
    //}
    routeDataIndividual(ack, destination, npdu, priority, _deviceObj.individualAddress(), kLocalIfIndex);
}

void NetworkLayerCoupler::dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    // If the group address is in the filter table, then we route it to the primary side too
    if (isGroupAddressInFilterTable(destination))
    {
        _netLayerEntities[kPrimaryIfIndex].sendDataRequest(npdu, ack, destination, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
    }
    // We send it to our sub line in any case
    _netLayerEntities[kSecondaryIfIndex].sendDataRequest(npdu, ack, destination, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
}

void NetworkLayerCoupler::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    CemiFrame tmpFrame(tpdu.frame());

    _netLayerEntities[kPrimaryIfIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
    _netLayerEntities[kSecondaryIfIndex].sendDataRequest(tmpFrame.npdu(), ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
}

void NetworkLayerCoupler::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());


    CemiFrame tmpFrame(tpdu.frame());

    // for closed media like TP1 and IP
    bool isClosedMedium = (_netLayerEntities[kPrimaryIfIndex].mediumType() == DptMedium::KNX_TP1) || (_netLayerEntities[kPrimaryIfIndex].mediumType() == DptMedium::KNX_IP);
    SystemBroadcast broadcastType = (isClosedMedium && isApciSystemBroadcast(tpdu.apdu()) ? Broadcast : SysBroadcast);
    _netLayerEntities[kPrimaryIfIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, broadcastType);

    isClosedMedium = (_netLayerEntities[kSecondaryIfIndex].mediumType() == DptMedium::KNX_TP1) || (_netLayerEntities[kSecondaryIfIndex].mediumType() == DptMedium::KNX_IP);
    broadcastType = (isClosedMedium && isApciSystemBroadcast(tmpFrame.apdu()) ? Broadcast : SysBroadcast);
    println(broadcastType);
    _netLayerEntities[kSecondaryIfIndex].sendDataRequest(tmpFrame.npdu(), ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, broadcastType);
}

#ifdef KNX_TUNNELING
bool NetworkLayerCoupler::isTunnelAddress(uint16_t destination)
{
    // tunnels are managed within the IpDataLinkLayer - kPrimaryIfIndex
    return _netLayerEntities[kPrimaryIfIndex].dataLinkLayer().isTunnelAddress(destination);
}
#endif