#include "network_layer.h"
#include "network_layer_entity.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "data_link_layer.h"
#include "bits.h"

NetworkLayerEntity::NetworkLayerEntity(NetworkLayer &netLayer, uint8_t entityIndex) : _netLayer(netLayer), _entityIndex(entityIndex)
{
}

void NetworkLayerEntity::dataLinkLayer(DataLinkLayer& layer)
{
    _dataLinkLayer = &layer;
}

void NetworkLayerEntity::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    npdu.frame().sourceInterface(_entityIndex);
    _netLayer.dataIndication(ack, addrType, destination, format, npdu, priority, source);
}

void NetworkLayerEntity::dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    npdu.frame().sourceInterface(_entityIndex);
    _netLayer.dataConfirm(ack, addressType, destination, format, priority, source, npdu, status);
}

void NetworkLayerEntity::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    npdu.frame().sourceInterface(_entityIndex);
    _netLayer.broadcastIndication(ack, format, npdu, priority, source);
}

void NetworkLayerEntity::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    npdu.frame().sourceInterface(_entityIndex);
    _netLayer.broadcastConfirm(ack, format, priority, source, npdu, status);
}

void NetworkLayerEntity::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
{
    npdu.frame().sourceInterface(_entityIndex);
    _netLayer.systemBroadcastIndication(ack, format, npdu, priority, source);
}

void NetworkLayerEntity::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
{
    npdu.frame().sourceInterface(_entityIndex);
    _netLayer.systemBroadcastConfirm(ack, format, priority, source, npdu, status);
}

void NetworkLayerEntity::sendDataRequest(NPDU &npdu, AckType ack, uint16_t destination, Priority priority, AddressType addrType, SystemBroadcast systemBroadcast)
{
    FrameFormat frameFormat = npdu.octetCount() > 15 ? ExtendedFrame : StandardFrame;

    if (systemBroadcast == Broadcast)
        _dataLinkLayer->dataRequest(ack, addrType, destination, frameFormat, priority, npdu);
    else
        _dataLinkLayer->systemBroadcastRequest(ack, frameFormat, priority, npdu);
}
