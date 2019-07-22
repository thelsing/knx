#pragma once

#include <stdint.h>
#include "device_object.h"
#include "address_table_object.h"
#include "knx_types.h"
#include "network_layer.h"

class DataLinkLayer
{
  public:
    DataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, NetworkLayer& layer,
                  Platform& platform);

    // from network layer
    void dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format,
                     Priority priority, NPDU& npdu);
    void systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu);
    virtual void loop() = 0;
    virtual void enabled(bool value) = 0;
    virtual bool enabled() const = 0;

  protected:
    void frameRecieved(CemiFrame& frame);
    void dataConReceived(CemiFrame& frame, bool success);
    bool sendTelegram(NPDU& npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority);
    virtual bool sendFrame(CemiFrame& frame) = 0;
    uint8_t* frameData(CemiFrame& frame);
    DeviceObject& _deviceObject;
    AddressTableObject& _groupAddressTable;
    NetworkLayer& _networkLayer;
    Platform& _platform;
};
