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
    if ((_deviceObj.induvidualAddress() & 0x00FF) == 0x00)
    {
        if ((_deviceObj.induvidualAddress() & 0x0F00) == 0x0)
        {
            // Device is a backbone coupler -> individual address: x.0.0
        }
        else
        {
            // Device is a line coupler -> individual address: x.y.0
        }
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

void NetworkLayerCoupler::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    // TODO: implement routing

    // Check if received frame is for us, we are a normal device in this case
    if (addrType == InduvidualAddress && destination == _deviceObj.induvidualAddress())
    {
        _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
        return;
    }

    // group-address type
    if (destination != 0)
    {
        _transportLayer.dataGroupIndication(destination, hopType, priority, source, npdu.tpdu());
        return;
    }

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
        // group-address type
        if (destination != 0)
        {
            _transportLayer.dataGroupConfirm(ack, source, destination, hopType, priority, npdu.tpdu(), status);
            return;
        }
    }

    // Do not process the frame any further if it was a routed frame sent from network layer
}

void NetworkLayerCoupler::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    uint8_t sourceInterfaceIndex = npdu.frame().sourceInterface();
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

    // TODO: implement routing
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

    // TODO: implement routing
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
