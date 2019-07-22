#pragma once

#include <stdint.h>
#include "data_link_layer.h"
#include "ip_parameter_object.h"

class IpDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;

  public:
    IpDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, IpParameterObject& ipParam, NetworkLayer& layer,
                    Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;

  private:
    bool _enabled = false;
    bool sendFrame(CemiFrame& frame);
    bool sendBytes(uint8_t* buffer, uint16_t length);

    IpParameterObject& _ipParameters;
};