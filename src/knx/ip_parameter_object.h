#pragma once

#include "config.h"
#ifdef USE_IP
#include "interface_object.h"
#include "device_object.h"
#include "platform.h"

class IpParameterObject : public InterfaceObject
{
  public:
    IpParameterObject(DeviceObject& deviceObject, Platform& platform);
    uint32_t multicastAddress() const;
    uint8_t ttl() const;

  private:
    DeviceObject& _deviceObject;
    Platform& _platform;
};
#endif