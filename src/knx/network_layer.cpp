#include "network_layer.h"
#include "device_object.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"

NetworkLayer::NetworkLayer(DeviceObject &deviceObj, TransportLayer& layer) :
    _deviceObj(deviceObj),
    _transportLayer(layer)
{
    _hopCount = _deviceObj.defaultHopCount();

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

uint8_t NetworkLayer::hopCount() const
{
    return _hopCount;
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
