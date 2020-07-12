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

    virtual NetworkLayerEntity& getEntity(uint8_t interfaceIndex) override;

    // from transport layer
    virtual void dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    virtual void dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    virtual void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    virtual void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) override;

  private:
    // Support only physical interface for normal devices
    NetworkLayerEntity _netLayerEntities[1];
};
