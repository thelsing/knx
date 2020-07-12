#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "npdu.h"
#include "transport_layer.h"
#include "network_layer_entity.h"

class DeviceObject;

class NetworkLayer
{
    friend class NetworkLayerEntity;

  public:
    NetworkLayer(DeviceObject& deviceObj, TransportLayer& layer);

    virtual NetworkLayerEntity& getEntity(uint8_t interfaceIndex) = 0;

    uint8_t hopCount() const;

    // from transport layer
    virtual void dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) = 0;
    virtual void dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) = 0;
    virtual void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) = 0;
    virtual void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) = 0;

  protected:
    DeviceObject& _deviceObj;

    // from entities
    virtual void dataIndication(AckType ack, AddressType addType, uint16_t destination, FrameFormat format, NPDU& npdu,
                                Priority priority, uint16_t source);
    virtual void dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority,
                             uint16_t source, NPDU& npdu, bool status);
    virtual void broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                                     Priority priority, uint16_t source);
    virtual void broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status);
    virtual void systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                                           Priority priority, uint16_t source);
    virtual void systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status);

  private:
    TransportLayer& _transportLayer;
    uint8_t _hopCount; // Network Layer Parameter hop_count for the device's own outgoing frames (default value from PID_ROUTING_COUNT)
};
