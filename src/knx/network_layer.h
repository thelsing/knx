#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "npdu.h"
#include "transport_layer.h"
class DataLinkLayer;

class NetworkLayer
{
  public:
    NetworkLayer(TransportLayer& layer);

    void dataLinkLayer(DataLinkLayer& layer);
    uint8_t hopCount() const;
    void hopCount(uint8_t value);

    // from data link layer
    void dataIndication(AckType ack, AddressType addType, uint16_t destination, FrameFormat format, NPDU& npdu,
                        Priority priority, uint16_t source);
    void dataConfirm(AckType ack, AddressType addressType, uint16_t destination, FrameFormat format, Priority priority,
                     uint16_t source, NPDU& npdu, bool status);
    void systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                                   Priority priority, uint16_t source);
    void systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status);

    // from transport layer
    void dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu);
    void dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu);
    void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu);
    void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu);

  private:
    void sendDataRequest(TPDU& tpdu, HopCountType hopType, AckType ack, uint16_t destination, Priority priority, AddressType addrType);
    uint8_t _hopCount = 6;
    DataLinkLayer* _dataLinkLayer = 0;
    TransportLayer& _transportLayer;
};
