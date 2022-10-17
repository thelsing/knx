#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "npdu.h"
#include "transport_layer.h"
#include "network_layer_entity.h"
#include "network_layer.h"

class DeviceObject;

class NetworkLayerDevice : public NetworkLayer
{
    friend class NetworkLayerEntity;

  public:
    NetworkLayerDevice(DeviceObject& deviceObj, TransportLayer& layer);

    NetworkLayerEntity& getInterface();

    // from transport layer
    void dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    void dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) override;

  private:
    // from entities
    void dataIndication(AckType ack, AddressType addType, uint16_t destination, FrameFormat format, NPDU& npdu,
                        Priority priority, uint16_t source, uint8_t srcIfIdx) override;
    void dataConfirm(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, Priority priority,
                     uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx) override;
    void broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                             Priority priority, uint16_t source, uint8_t srcIfIdx) override;
    void broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx) override;
    void systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                                   Priority priority, uint16_t source, uint8_t srcIfIdx) override;
    void systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx) override;

    // Support only a single physical interface for normal devices
    NetworkLayerEntity _netLayerEntities[1];

    static constexpr uint8_t kInterfaceIndex = 0;
};
