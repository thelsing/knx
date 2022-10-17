#pragma once

#include "config.h"
#ifdef USE_IP

#include <stdint.h>
#include "data_link_layer.h"
#include "ip_parameter_object.h"

class IpDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;

  public:
    IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam, NetworkLayerEntity& netLayerEntity,
                    Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;
    DptMedium mediumType() const override;

  private:
    bool _enabled = false;
    bool sendFrame(CemiFrame& frame);
    bool sendBytes(uint8_t* buffer, uint16_t length);

    IpParameterObject& _ipParameters;
};
#endif
