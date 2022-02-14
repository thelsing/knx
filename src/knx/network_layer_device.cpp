#include "network_layer_device.h"
#include "device_object.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"

NetworkLayerDevice::NetworkLayerDevice(DeviceObject &deviceObj, TransportLayer& layer) :
    NetworkLayer(deviceObj, layer),
    _netLayerEntities { {*this, kInterfaceIndex} }
{
}

NetworkLayerEntity& NetworkLayerDevice::getInterface()
{
    return _netLayerEntities[kInterfaceIndex];
}

void NetworkLayerDevice::dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
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
    _netLayerEntities[kInterfaceIndex].sendDataRequest(npdu, ack, destination, _deviceObj.individualAddress(), priority, IndividualAddress, Broadcast);
}

void NetworkLayerDevice::dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[kInterfaceIndex].sendDataRequest(npdu, ack, destination, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
}

void NetworkLayerDevice::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[kInterfaceIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, Broadcast);
}

void NetworkLayerDevice::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu)
{
    // for closed media like TP1 and IP
    bool isClosedMedium = (_netLayerEntities[kInterfaceIndex].mediumType() == DptMedium::KNX_TP1) || (_netLayerEntities[kInterfaceIndex].mediumType() == DptMedium::KNX_IP);
    SystemBroadcast broadcastType = (isClosedMedium && isApciSystemBroadcast(tpdu.apdu()) ? Broadcast : SysBroadcast);

    NPDU& npdu = tpdu.frame().npdu();

    if (hopType == UnlimitedRouting)
        npdu.hopCount(7);
    else
        npdu.hopCount(hopCount());

    _netLayerEntities[kInterfaceIndex].sendDataRequest(npdu, ack, 0, _deviceObj.individualAddress(), priority, GroupAddress, broadcastType);
}

void NetworkLayerDevice::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;

    if (addrType == IndividualAddress)
    {
        if (destination != _deviceObj.individualAddress())
            return;

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

void NetworkLayerDevice::dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    if (addressType == IndividualAddress)
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

void NetworkLayerDevice::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    DptMedium mediumType = _netLayerEntities[srcIfIdx].mediumType();

    // for closed media like TP1 and IP there is no system broadcast
    // however we must be able to access those APCI via broadcast mode
    // so we "translate" it to system broadcast like a coupler does when routing
    // between closed and open media
    if ( ((mediumType == DptMedium::KNX_TP1) || (mediumType == DptMedium::KNX_IP)) &&
          isApciSystemBroadcast(npdu.tpdu().apdu()))
    {
        npdu.frame().systemBroadcast(SysBroadcast);
        _transportLayer.dataSystemBroadcastIndication(hopType, priority, source, npdu.tpdu());
        return;
    }

    _transportLayer.dataBroadcastIndication(hopType, priority, source, npdu.tpdu());
}

void NetworkLayerDevice::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataBroadcastConfirm(ack, hopType, priority, npdu.tpdu(), status);
}

void NetworkLayerDevice::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataSystemBroadcastIndication(hopType, priority, source, npdu.tpdu());
}

void NetworkLayerDevice::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx)
{
    HopCountType hopType = npdu.hopCount() == 7 ? UnlimitedRouting : NetworkLayerParameter;
    _transportLayer.dataSystemBroadcastConfirm(ack, hopType, npdu.tpdu(), priority, status);
}
