#pragma once

#include <stdint.h>
#include "device_object.h"
#include "address_table_object.h"
#include "ip_parameter_object.h"
#include "knx_types.h"
#include "network_layer.h"

class DataLinkLayer
{
public:
    DataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, IpParameterObject& ipParam, NetworkLayer& layer, 
        Platform& platform);

    // from network layer
    void dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format, 
        Priority priority, NPDU& npdu);
    void systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu);
    void loop();
    void enabled(bool value);
    bool enabled() const;
private:
    bool _enabled = false;
    bool sendPacket(NPDU &npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority);
    bool sendBytes(uint8_t* buffer, uint16_t length);

    DeviceObject& _deviceObject;
    AddressTableObject& _groupAddressTable;
    IpParameterObject& _ipParameters;
    NetworkLayer& _networkLayer;
    Platform& _platform;
};