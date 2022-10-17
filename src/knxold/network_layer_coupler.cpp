#include "network_layer_coupler.h"
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

bool NetworkLayerCoupler::isRoutedIndividualAddress(uint16_t individualAddress)
{
    // TODO: ACKs for frames with individual addresses of the sub line (secondary I/F)
    // Check spec. about his
    // See PID_MAIN_LCCONFIG/PID_SUB_LCCONFIG: PHYS_IACK
    // 0 = not used
    // 1 = normal mode (all frames that will be routed or that are addressed to the Coupler itself will be acknowledged)
    // 2 = all frames will be acknowledged (useful only to avoid the repetitions of misrouted frames)
    // 3 = all frames on point-to-point connectionless – or connection-oriented communication mode shall be negatively acknowledge (NACK).
    //     This shall serve for protection purposes. (It is useful to prevent all parameterisation in one Subnetwork; the Coupler shall be protected
    //     too. A typical use case is the protection of a Subnetwork that is located outside a building)

    // Also ACK for our own individual address
    if (individualAddress == _deviceObj.individualAddress())
        return true;

    // use 2 for now
    return true;
}

void NetworkLayerCoupler::sendMsgHopCount(AckType ack, AddressType addrType, uint16_t destination, NPDU& npdu, Priority priority,
                                          SystemBroadcast broadcastType, uint8_t sourceInterfaceIndex, uint16_t source)
{
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
    uint8_t interfaceIndex = (sourceInterfaceIndex == kSecondaryIfIndex) ? kPrimaryIfIndex : kSecondaryIfIndex;
    if (sourceInterfaceIndex == 0)
        print("Routing from P->S: ");
    else
        print("Routing from S->P: ");
    print(source, HEX); print(" -> "); print(destination, HEX);
    print(" - ");
    npdu.frame().apdu().printPDU();
    _netLayerEntities[interfaceIndex].sendDataRequest(npdu, ack, destination, source, priority, addrType, broadcastType);
}

// TODO: for later: improve by putting routing algorithms in its own class/functions and only instantiate required algorithm (line vs. coupler)
// TODO: we could also do the sanity checks here, i.e. check if sourceAddress is really coming in from correct srcIfIdx, etc. (see PID_COUPL_SERV_CONTROL: EN_SNA_INCONSISTENCY_CHECK)
void NetworkLayerCoupler::routeDataIndividual(AckType ack, uint16_t destination, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIndex)
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
    uint16_t ZS = destination & 0xFF00;                        // destination subnetwork address (area + line)
    uint16_t Z = destination & 0xF000;                         // destination area address
    uint16_t D = destination & 0x00FF;                         // destination device address (without subnetwork part)
    uint16_t SD = destination & 0x0FFF;                        // destination device address (with line part, but without area part)

    if (_couplerType == LineCoupler)
    {
        // Main line to sub line routing
        if (srcIfIndex == kPrimaryIfIndex)
        {
            if (ZS != ownSNA)
            {
                // IGNORE_TOTALLY
                return;
            }

            if (D == 0)
            {
                // FORWARD_LOCALLY
                HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
                _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
            }
            else
            {   // ROUTE_XXX
                sendMsgHopCount(ack, AddressType::IndividualAddress, destination, npdu, priority, Broadcast, srcIfIndex, source);
            }
            return;
        }

        // Sub line to main line routing
        if (srcIfIndex == kSecondaryIfIndex)
        {
            if (ZS != ownSNA)
            {
                // ROUTE_XXX
                sendMsgHopCount(ack, AddressType::IndividualAddress, destination, npdu, priority, Broadcast, srcIfIndex, source);
            }
            else if (D == 0)
            {
                // FORWARD_LOCALLY
                HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
                _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
            }
            else
            {
                // IGNORE_TOTALLY
            }
            return;
        }

        // Local to main or sub line
        if (srcIfIndex == kLocalIfIndex)
        {
            // if destination is not within our subnet then send via primary interface, else via secondary interface
            uint8_t destIfidx = (ZS != ownSNA) ? kPrimaryIfIndex : kSecondaryIfIndex;
            _netLayerEntities[destIfidx].sendDataRequest(npdu, ack, destination, source, priority, AddressType::IndividualAddress, Broadcast);
            return;
        }
    }

    if (_couplerType == BackboneCoupler)
    {
        // Backbone line to main line routing
        if (srcIfIndex == kPrimaryIfIndex)
        {
            if (Z != ownAA)
            {
                // IGNORE_TOTALLY
                return;
            }

            if (SD == 0)
            {
                // FORWARD_LOCALLY
                HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
                _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
            }
            else
            {
                // ROUTE_XXX
                sendMsgHopCount(ack, AddressType::IndividualAddress, destination, npdu, priority, Broadcast, srcIfIndex, source);
            }
            return;
        }

        // Main line to backbone line routing
        if (srcIfIndex == kSecondaryIfIndex)
        {
            if (Z != ownAA)
            {
                // ROUTE_XXX
                sendMsgHopCount(ack, AddressType::IndividualAddress, destination, npdu, priority, Broadcast, srcIfIndex, source);
            }
            else if(SD == 0)
            {
                // FORWARD_LOCALLY
                HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
                _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
            }
            else
            {
                // IGNORE_TOTALLY
            }
            return;
        }

        // Local to main or sub line
        if (srcIfIndex == kLocalIfIndex)
        {
            // if destination is not within our area then send via primary interface, else via secondary interface
            uint8_t destIfidx = (Z != ownAA) ? kPrimaryIfIndex : kSecondaryIfIndex;
            _netLayerEntities[destIfidx].sendDataRequest(npdu, ack, destination, source, priority, AddressType::IndividualAddress, Broadcast);
            return;
        }
    }
}

void NetworkLayerCoupler::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    // routing for individual addresses
    if (addrType == IndividualAddress)
    {
        routeDataIndividual(ack, destination, npdu, priority, source, srcIfIdx);
        return;
    }

    // routing for group addresses
    // TODO: check new AN189
    // "AN189 only makes that group messages with hop count 7 cannot bypass the Filter Table unfiltered,
    // what made the Security Proxy(AN192) useless; now, hc 7 Telegrams are filtered as any other and the value is decremented.
    if (isGroupAddressInFilterTable(destination))
    {
        // ROUTE_XXX
        sendMsgHopCount(ack, addrType, destination, npdu, priority, Broadcast, srcIfIdx, source);
        return;
    }
    else
    {
        // IGNORE_TOTALLY
        return;
    }

    println("Unhandled routing case! Should not happen!");
}

void NetworkLayerCoupler::dataConfirm(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
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

    // Route to other interface
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
    // Route to other interface
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

    _netLayerEntities[kPrimaryIfIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
    _netLayerEntities[kSecondaryIfIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
}

void NetworkLayerCoupler::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[kPrimaryIfIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, SysBroadcast);
    _netLayerEntities[kSecondaryIfIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, SysBroadcast);
}
