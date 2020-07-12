#include "network_layer_coupler.h"
#include "device_object.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"

NetworkLayerCoupler::NetworkLayerCoupler(DeviceObject &deviceObj, TransportLayer& layer) :
    NetworkLayer(deviceObj, layer),
    _netLayerEntities { {*this, 0}, {*this, 1} }
{
}

NetworkLayerEntity& NetworkLayerCoupler::getEntity(uint8_t interfaceIndex)
{
    return _netLayerEntities[interfaceIndex];
}

void NetworkLayerCoupler::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    // Check if received frame is for us, we are a normal device in this case
    if (addrType == InduvidualAddress && destination == _deviceObj.induvidualAddress())
    {
        NetworkLayer::dataIndication(ack, addrType, destination, format, npdu, priority, source);
        // Do not process the frame any further
        return;
    }

    // TODO: implement routing
}

void NetworkLayerCoupler::dataConfirm(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.induvidualAddress())
    {
        NetworkLayer::dataConfirm(ack, addrType, destination, format, priority, source, npdu, status);
    }
    // Do not process the frame any further
}

void NetworkLayerCoupler::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    NetworkLayer::broadcastIndication(ack, format, npdu, priority, source);

    // TODO: implement routing
}

void NetworkLayerCoupler::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.induvidualAddress())
    {
        NetworkLayer::broadcastConfirm(ack, format, priority, source, npdu, status);
    }
    // Do not process the frame any further
}

void NetworkLayerCoupler::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    NetworkLayer::systemBroadcastIndication(ack, format, npdu, priority, source);

    // TODO: implement routing
}

void NetworkLayerCoupler::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    // Check if received frame is an echo from our sent frame, we are a normal device in this case
    if (source == _deviceObj.induvidualAddress())
    {
        NetworkLayer::systemBroadcastConfirm(ack, format, priority, source, npdu, status);
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
