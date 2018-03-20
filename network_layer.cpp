#include "network_layer.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "data_link_layer.h"
#include "bits.h"

NetworkLayer::NetworkLayer(TransportLayer& layer): _transportLayer(layer)
{

}

void NetworkLayer::dataLinkLayer(DataLinkLayer& layer)
{
    _dataLinkLayer = &layer;
}

uint8_t NetworkLayer::hopCount() const
{
    return _hopCount;
}

void NetworkLayer::hopCount(uint8_t value)
{
    _hopCount = value & 0x7;
}

void NetworkLayer::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    if (addrType == InduvidualAddress)
    {
        //if (npdu.octetCount() > 0)
        //{
        //    print.print("<- NL  ");
        //    npdu.frame().apdu().printPDU();
        //}
        _transportLayer.dataIndividualIndication(destination, hopType, priority, source, npdu.tpdu());
        return;
    }
    // group-address type
    if (destination != 0)
    {
        _transportLayer.dataGroupIndication(destination, hopType, priority, source, npdu.tpdu());
        return;
    }
    // destination == 0
    _transportLayer.dataBroadcastIndication(hopType, priority, source, npdu.tpdu());

}

void NetworkLayer::dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    if (addressType == InduvidualAddress)
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
    // destination == 0
    _transportLayer.dataBroadcastConfirm(ack, hopType, priority, npdu.tpdu(), status);
}

void NetworkLayer::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataBroadcastIndication(hopType, priority, source, npdu.tpdu());
}

void NetworkLayer::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataBroadcastConfirm(ack, hopType, priority, npdu.tpdu(), status);
}

void NetworkLayer::dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    //if (tpdu.apdu().length() > 0)
    //{
    //    print.print("-> NL  ");
    //    tpdu.apdu().printPDU();
    //}
    sendDataRequest(tpdu, hopType, ack, destination, priority, InduvidualAddress);
}

void NetworkLayer::sendDataRequest(TPDU &tpdu, HopCountType hopType, AckType ack, uint16_t destination, Priority priority, AddressType addrType)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(_hopCount);

    FrameFormat frameFormat = npdu.octetCount() > 15 ? ExtendedFrame : StandardFrame;

    _dataLinkLayer->dataRequest(ack, addrType, destination, frameFormat, priority, npdu);
}

void NetworkLayer::dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    sendDataRequest(tpdu, hopType, ack, destination, priority, GroupAddress);
}

void NetworkLayer::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    sendDataRequest(tpdu, hopType, ack, 0, priority, GroupAddress);
}

void NetworkLayer::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(_hopCount);

    FrameFormat frameFormat = npdu.octetCount() > 15 ? ExtendedFrame : StandardFrame;

    _dataLinkLayer->systemBroadcastRequest(ack, frameFormat, priority, npdu);
}
