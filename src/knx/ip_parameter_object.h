#pragma once

#include "config.h"
#ifdef USE_IP
#include "interface_object.h"
#include "device_object.h"
#include "platform.h"

#define KNXIP_MULTICAST_PORT 3671

class IpParameterObject : public InterfaceObject
{
    public:
        IpParameterObject(DeviceObject& deviceObject, Platform& platform);
        uint16_t* additionalIndivualAddresses(uint8_t& numAddresses);
    private:
        DeviceObject& _deviceObject;
        Platform& _platform;
};
#endif