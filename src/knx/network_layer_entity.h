#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "npdu.h"

class DataLinkLayer;
class NetworkLayer;

class NetworkLayerEntity
{
    friend class NetworkLayerCoupler;
    friend class NetworkLayerDevice;

  public:
    NetworkLayerEntity(NetworkLayer &netLayer, uint8_t entityIndex);

    void dataLinkLayer(DataLinkLayer& layer);
    DataLinkLayer& dataLinkLayer();

    DptMedium mediumType() const;
    uint8_t getEntityIndex();

    // from data link layer
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
    // From network layer
    void sendDataRequest(NPDU& npdu, AckType ack, uint16_t destination, uint16_t source, Priority priority, AddressType addrType, SystemBroadcast systemBroadcast);

    DataLinkLayer* _dataLinkLayer = 0;
    NetworkLayer& _netLayer;
    uint8_t _entityIndex;
};
