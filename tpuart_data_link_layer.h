#pragma once

#include <stdint.h>
#include "data_link_layer.h"

class TpUartDataLinkLayer: public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;
public:
    TpUartDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, NetworkLayer& layer,
        Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;
private:
    bool _enabled = false;
    bool sendFrame(CemiFrame& frame);
    bool sendBytes(uint8_t* buffer, uint16_t length);
};