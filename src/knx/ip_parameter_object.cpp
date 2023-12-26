#include "ip_parameter_object.h"
#ifdef USE_IP
#include "device_object.h"
#include "platform.h"
#include "bits.h"
#include "data_property.h"
#include "callback_property.h"

// 224.0.23.12
#define DEFAULT_MULTICAST_ADDR ((uint32_t)0xE000170C)

IpParameterObject::IpParameterObject(DeviceObject& deviceObject, Platform& platform): _deviceObject(deviceObject),
    _platform(platform)
{
    Property* properties[] =
    {
        new DataProperty(PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_IP_PARAMETER),
        new DataProperty(PID_PROJECT_INSTALLATION_ID, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv3),
        new CallbackProperty<IpParameterObject>(this, PID_KNX_INDIVIDUAL_ADDRESS, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv3,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            {
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }
                // TODO: get property of deviceobject and use it
                pushWord(io->_deviceObject.individualAddress(), data);
                return 1;
            },
            [](IpParameterObject* io, uint16_t start, uint8_t count, const uint8_t* data) -> uint8_t 
            { 
                io->_deviceObject.individualAddress(getWord(data));
                return 1; 
            }),
        new DataProperty(PID_CURRENT_IP_ASSIGNMENT_METHOD, false, PDT_UNSIGNED_CHAR, 0, ReadLv3 | WriteLv3),
        new DataProperty(PID_IP_ASSIGNMENT_METHOD, true, PDT_UNSIGNED_CHAR, 1, ReadLv3 | WriteLv3),
        new DataProperty(PID_IP_CAPABILITIES, true, PDT_BITSET8, 0, ReadLv3 | WriteLv1),    // must be set by application due to capabilities of the used ip stack
        new CallbackProperty<IpParameterObject>(this, PID_CURRENT_IP_ADDRESS, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                pushInt(htonl(io->_platform.currentIpAddress()), data);
                return 1;
            }),
        new CallbackProperty<IpParameterObject>(this, PID_CURRENT_SUBNET_MASK, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                pushInt(htonl(io->_platform.currentSubnetMask()), data);
                return 1;
            }),
        new CallbackProperty<IpParameterObject>(this, PID_CURRENT_DEFAULT_GATEWAY, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                pushInt(htonl(io->_platform.currentDefaultGateway()), data);
                return 1;
            }),
        new DataProperty(PID_IP_ADDRESS, true, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv3),
        new DataProperty(PID_SUBNET_MASK, true, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv3),
        new DataProperty(PID_DEFAULT_GATEWAY, true, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv3),
        new CallbackProperty<IpParameterObject>(this, PID_MAC_ADDRESS, false, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                io->_platform.macAddress(data);
                return 1;
            }),
        new CallbackProperty<IpParameterObject>(this, PID_SYSTEM_SETUP_MULTICAST_ADDRESS, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                pushInt(DEFAULT_MULTICAST_ADDR, data);
                return 1;
            }),
        new DataProperty(PID_ROUTING_MULTICAST_ADDRESS, true, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv3, DEFAULT_MULTICAST_ADDR),
        new DataProperty(PID_TTL, true, PDT_UNSIGNED_CHAR, 1, ReadLv3 | WriteLv3, (uint8_t)16),
        new CallbackProperty<IpParameterObject>(this, PID_KNXNETIP_DEVICE_CAPABILITIES, false, PDT_BITSET16, 1, ReadLv3 | WriteLv0,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                pushWord(0x1, data);
                return 1;
            }),
        new DataProperty(PID_FRIENDLY_NAME, true, PDT_UNSIGNED_CHAR, 30, ReadLv3 | WriteLv3)
    };
    initializeProperties(sizeof(properties), properties);
}

#endif
