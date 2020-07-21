#include "network_layer_coupler.h"
#include "device_object.h"
#include "router_object.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"

NetworkLayerCoupler::NetworkLayerCoupler(DeviceObject &deviceObj,
                                         TransportLayer& layer) :
    NetworkLayer(deviceObj, layer),
    _netLayerEntities { {*this, 0}, {*this, 1} }
{
    // Check coupler mode
    if ((_deviceObj.induvidualAddress() & 0x00FF) == 0x00)
    {
        // Device is a router
        // Check if line coupler or backbone coupler
        if ((_deviceObj.induvidualAddress() & 0x0F00) == 0x0)
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

NetworkLayerEntity& NetworkLayerCoupler::getEntity(uint8_t interfaceIndex)
{
    return _netLayerEntities[interfaceIndex];
}

void NetworkLayerCoupler::rtObjPrimary(RouterObject& rtObjPrimary)
{
    _rtObjPrimary = &rtObjPrimary;
}

void NetworkLayerCoupler::rtObjSecondary(RouterObject& rtObjSecondary)
{
    _rtObjSecondary = &rtObjSecondary;
}

void NetworkLayerCoupler::routeMsgHopCount(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority,
                                       SystemBroadcast broadcastType, uint8_t sourceInterfaceIndex)
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

    // Use other interface
    uint8_t interfaceIndex = (sourceInterfaceIndex == 1) ? 0 : 1;

    _netLayerEntities[interfaceIndex].sendDataRequest(npdu, ack, destination, priority, addrType, broadcastType);
}

void NetworkLayerCoupler::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    uint8_t srcIfIndex = npdu.frame().sourceInterface();       // Source interface (0 = primary, 1 = secondary)
    uint16_t ownSNA = _deviceObj.induvidualAddress() & 0xFF00; // Own subnetwork address (area + line)
    uint16_t ownAA = _deviceObj.induvidualAddress() & 0xF000;  // Own area address
    uint16_t ZS = destination & 0xFF00;                        // destination subnetwork address (area + line)
    uint16_t Z = destination & 0xF000;                         // destination area address
    uint16_t D = _deviceObj.induvidualAddress() & 0x00FF;      // Own device address (without subnetwork part)
    uint16_t SD = _deviceObj.induvidualAddress() & 0x0FFF;     // Own device address (with line part, but without area part)

    // routing for individual addresses
    if (addrType == InduvidualAddress)
    {
        if (_couplerType == LineCoupler)
        {
            // Main line to sub line routing
            if (srcIfIndex == 0)
            {
                if (ZS != ownSNA)
                    return; // IGNORE_TOTALLY

                if (D == 0)
                {
                    // FORWARD_LOCALLY
                    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
                    _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
                }
                else
                {
                    routeMsgHopCount(ack, addrType, destination, format, npdu, priority, Broadcast, srcIfIndex);
                }
                return;
            }

            // Sub line to main line routing
            if (srcIfIndex == 1)
            {
                if (ZS != ownSNA)
                    routeMsgHopCount(ack, addrType, destination, format, npdu, priority, Broadcast, srcIfIndex);
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
        }

        if (_couplerType == BackboneCoupler)
        {
            // Backbone line to main line routing
            if (srcIfIndex == 0)
            {
                if (Z != ownAA)
                    return; // IGNORE_TOTALLY

                if (SD == 0)
                {
                    // FORWARD_LOCALLY
                    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
                    _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
                }
                else
                {
                    routeMsgHopCount(ack, addrType, destination, format, npdu, priority, Broadcast, srcIfIndex);
                }
                return;
            }

            // Main line to backbone line routing
            if (srcIfIndex == 1)
            {
                if (Z != ownAA)
                    routeMsgHopCount(ack, addrType, destination, format, npdu, priority, Broadcast, srcIfIndex);
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
        }
    }

    // routing for group addresses
    // TODO: check new AN189
    // "AN189 only makes that group messages with hop count 7 cannot bypass the Filter Table unfiltered,
    // what made the Security Proxy(AN192) useless; now, hc 7 Telegrams are filtered as any other and the value is decremented.
    if (_rtObjSecondary->isGroupAddressInFilterTable(destination))
    {
        routeMsgHopCount(ack, addrType, destination, format, npdu, priority, Broadcast, srcIfIndex);
        return;
    }
    else
    {
        // IGNORE_TOTALLY
        return;
    }

    println("Unhandled routing case! Should not happen!");
}

void NetworkLayerCoupler::dataConfirm(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.induvidualAddress())
    {
        if (addrType == InduvidualAddress)
        {
            _transportLayer.dataIndividualConfirm(ack, destination, hopType, priority, npdu.tpdu(), status);
            return;
        }
        // else: we do not have any local group communication, so do not handle this
    }

    // Do not process the frame any further if it was a routed frame sent from network layer
}

void NetworkLayerCoupler::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    uint8_t sourceInterfaceIndex = npdu.frame().sourceInterface();

    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    // Send it to our local stack
    {
        DptMedium mediumType = getEntity(sourceInterfaceIndex).mediumType();

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
    routeMsgHopCount(ack, GroupAddress, 0, format, npdu, priority, Broadcast, npdu.frame().sourceInterface());
}

void NetworkLayerCoupler::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.induvidualAddress())
    {
         _transportLayer.dataBroadcastConfirm(ack, hopType, priority, npdu.tpdu(), status);
    }
    // Do not process the frame any further
}

void NetworkLayerCoupler::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataSystemBroadcastIndication(hopType, priority, source, npdu.tpdu());

    // Route to other interface
    routeMsgHopCount(ack, GroupAddress, 0, format, npdu, priority, SysBroadcast, npdu.frame().sourceInterface());
}

void NetworkLayerCoupler::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.induvidualAddress())
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
    _netLayerEntities[0].sendDataRequest(npdu, ack, destination, priority, InduvidualAddress, Broadcast);
}

void NetworkLayerCoupler::dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[0].sendDataRequest(npdu, ack, destination, priority, GroupAddress, Broadcast);
}

void NetworkLayerCoupler::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[0].sendDataRequest(npdu, ack, 0, priority, GroupAddress, Broadcast);
}

void NetworkLayerCoupler::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[0].sendDataRequest(npdu, ack, 0, priority, GroupAddress, SysBroadcast);
}
