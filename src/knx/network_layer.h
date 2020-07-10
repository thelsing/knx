#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "npdu.h"
#include "transport_layer.h"
#include "network_layer_entity.h"

class DeviceObject;

class NetworkLayer
{
  public:
    NetworkLayer(DeviceObject& deviceObj, TransportLayer& layer);

    NetworkLayerEntity& getEntity(uint8_t num);
    uint8_t hopCount() const;
    void hopCount(uint8_t value);

    // from transport layer
    void dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu);
    void dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu);
    void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu);
    void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu);

    // from entities
    void dataIndication(AckType ack, AddressType addType, uint16_t destination, FrameFormat format, NPDU& npdu,
                        Priority priority, uint16_t source);
    void dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority,
                     uint16_t source, NPDU& npdu, bool status);
    void broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                             Priority priority, uint16_t source);
    void broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status);
    void systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                                   Priority priority, uint16_t source);
    void systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status);

  private:
    uint8_t _hopCount = 6;

    // Support a maximum of two physical interfaces
    NetworkLayerEntity _netLayerEntities[2];

    TransportLayer& _transportLayer;
    DeviceObject& _deviceObj;
};
