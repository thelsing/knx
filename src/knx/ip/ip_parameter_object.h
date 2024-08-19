#pragma once

#include "../inteface_object/device_object.h"
#include "../platform/platform.h"

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
