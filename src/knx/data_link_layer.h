#pragma once

#include "config.h"

#include <stdint.h>
#include "device_object.h"
#include "knx_types.h"
#include "network_layer_entity.h"
#include "cemi_server.h"

class Platform;

class DataLinkLayer
{
  public:
    DataLinkLayer(DeviceObject& devObj, NetworkLayerEntity& netLayerEntity,
                  Platform& platform);

#ifdef USE_CEMI_SERVER
    // from tunnel
    void cemiServer(CemiServer& cemiServer);
    void dataRequestFromTunnel(CemiFrame& frame);
#endif

    // from network layer
    void dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format,
                     Priority priority, NPDU& npdu);
    void systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu);
    virtual void loop() = 0;
    virtual void enabled(bool value) = 0;
    virtual bool enabled() const = 0;
    virtual DptMedium mediumType() const = 0;

  protected:
    void frameRecieved(CemiFrame& frame);
    void dataConReceived(CemiFrame& frame, bool success);
    bool sendTelegram(NPDU& npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority, SystemBroadcast systemBroadcast);
    virtual bool sendFrame(CemiFrame& frame) = 0;
    uint8_t* frameData(CemiFrame& frame);
    DeviceObject& _deviceObject;
    NetworkLayerEntity& _networkLayerEntity;
    Platform& _platform;
#ifdef USE_CEMI_SERVER
    CemiServer* _cemiServer;
#endif    
};
