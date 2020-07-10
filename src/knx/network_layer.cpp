#include "network_layer.h"
#include "device_object.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"

NetworkLayer::NetworkLayer(DeviceObject &deviceObj, TransportLayer& layer) :
    _netLayerEntities { {*this, 0}, {*this, 1} },
    _transportLayer(layer),
    _deviceObj(deviceObj)
{
}

NetworkLayerEntity& NetworkLayer::getEntity(uint8_t num)
{
    return _netLayerEntities[num];
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

    // Only for devices which are not a coupler
    if (addrType == InduvidualAddress && destination != _deviceObj.induvidualAddress())
        return;

    // TODO: remove. getTSAP() will return 0 later anyway. Get rid of dependency to GAT
    //if (addrType == GroupAddress && !_groupAddressTable.contains(destination))
    //    return;

//        if (frame.npdu().octetCount() > 0)
//        {
//            _print("-> DLL ");
//            frame.apdu().printPDU();
//        }

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

void NetworkLayer::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataBroadcastIndication(hopType, priority, source, npdu.tpdu());
}

void NetworkLayer::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataBroadcastConfirm(ack, hopType, priority, npdu.tpdu(), status);
}

void NetworkLayer::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataSystemBroadcastIndication(hopType, priority, source, npdu.tpdu());
}

void NetworkLayer::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataSystemBroadcastConfirm(ack, hopType, npdu.tpdu(), priority, status);
}

void NetworkLayer::dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(_hopCount);

    //if (tpdu.apdu().length() > 0)
    //{
    //    print.print("-> NL  ");
    //    tpdu.apdu().printPDU();
    //}
    _netLayerEntities[0].sendDataRequest(npdu, ack, destination, priority, InduvidualAddress, Broadcast);
}

void NetworkLayer::dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(_hopCount);

    _netLayerEntities[0].sendDataRequest(npdu, ack, destination, priority, GroupAddress, Broadcast);
}

void NetworkLayer::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(_hopCount);

    _netLayerEntities[0].sendDataRequest(npdu, ack, 0, priority, GroupAddress, Broadcast);
}

void NetworkLayer::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(_hopCount);

    _netLayerEntities[0].sendDataRequest(npdu, ack, 0, priority, GroupAddress, SysBroadcast);
}
