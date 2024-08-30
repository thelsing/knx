#pragma once

#include "../interface_object/device_object.h"
#include "../platform/platform.h"

#define KNXIP_MULTICAST_PORT 3671

namespace Knx
{
    class IpParameterObject : public InterfaceObject
    {
        public:
            IpParameterObject(DeviceObject& deviceObject, Platform& platform);
            uint16_t* additionalIndivualAddresses(uint8_t& numAddresses);
            const char* name() override
            {
                return "IpParameterObject";
            }
        private:
            DeviceObject& _deviceObject;
            Platform& _platform;
    };
}