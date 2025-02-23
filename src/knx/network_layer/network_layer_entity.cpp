#include "network_layer_entity.h"

#include "../bits.h"
#include "../datalink_layer/data_link_layer.h"
#include "../transport_layer/tpdu.h"
#include "network_layer.h"

namespace Knx
{
    NetworkLayerEntity::NetworkLayerEntity(NetworkLayer& netLayer, uint8_t entityIndex)
        : _netLayer(netLayer), _entityIndex(entityIndex)
    {
    }

    void NetworkLayerEntity::dataLinkLayer(DataLinkLayer& layer)
    {
        _dataLinkLayer = &layer;
    }

    DataLinkLayer& NetworkLayerEntity::dataLinkLayer()
    {
        return *_dataLinkLayer;
    }

    NetworkLayer& NetworkLayerEntity::networkLayer()
    {
        return _netLayer;
    }

    DptMedium NetworkLayerEntity::mediumType() const
    {
        return _dataLinkLayer->mediumType();
    }

    uint8_t NetworkLayerEntity::getEntityIndex()
    {
        return _entityIndex;
    }

    void NetworkLayerEntity::dataIndication(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
    {
        _netLayer.dataIndication(ack, addrType, destination, format, npdu, priority, source, _entityIndex);
    }

    void NetworkLayerEntity::dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
    {
        _netLayer.dataConfirm(ack, addressType, destination, format, priority, source, npdu, status, _entityIndex);
    }

    void NetworkLayerEntity::broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
    {
        _netLayer.broadcastIndication(ack, format, npdu, priority, source, _entityIndex);
    }

    void NetworkLayerEntity::broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
    {
        _netLayer.broadcastConfirm(ack, format, priority, source, npdu, status, _entityIndex);
    }

    void NetworkLayerEntity::systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu, Priority priority, uint16_t source)
    {
        _netLayer.systemBroadcastIndication(ack, format, npdu, priority, source, _entityIndex);
    }

    void NetworkLayerEntity::systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status)
    {
        _netLayer.systemBroadcastConfirm(ack, format, priority, source, npdu, status, _entityIndex);
    }

    void NetworkLayerEntity::sendDataRequest(NPDU& npdu, AckType ack, uint16_t destination, uint16_t source, Priority priority, AddressType addrType, SystemBroadcast systemBroadcast, bool doNotRepeat)
    {
        FrameFormat frameFormat = npdu.octetCount() > 15 ? ExtendedFrame : StandardFrame;

        if (systemBroadcast == Broadcast)
            _dataLinkLayer->dataRequest(ack, addrType, destination, source, frameFormat, priority, npdu);
        else
            _dataLinkLayer->systemBroadcastRequest(ack, frameFormat, priority, npdu, source);
    }
} // namespace Knx