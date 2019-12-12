#include "ip_parameter_object.h"
#include "device_object.h"
#include "platform.h"
#include "bits.h"
#include "data_property.h"
#include "callback_property.h"

//224.0.23.12
#define DEFAULT_MULTICAST_ADDR 0xE000170C

IpParameterObject::IpParameterObject(DeviceObject& deviceObject, Platform& platform): _deviceObject(deviceObject),
    _platform(platform)
{
    Property* properties[] =
    {
        new DataProperty(PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, OT_IP_PARAMETER),
        new DataProperty(PID_PROJECT_INSTALLATION_ID, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv3),
        new CallbackProperty<IpParameterObject>(this, PID_KNX_INDIVIDUAL_ADDRESS, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv3,
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                pushWord(io->_deviceObject.induvidualAddress(), data);
                return 1;
            },
            [](IpParameterObject* io, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t 
            { 
                io->_deviceObject.induvidualAddress(getWord(data));
                return 1; 
            }),
        //55 PID_IP_ASSIGNMENT_METHOD  3 / 3
        //56 PID_IP_CAPABILITIES  3 / 1
        //57 PID_CURRENT_IP_ADDRESS  3 / x
        //58 PID_CURRENT_SUBNET_MASK  3 / x
        //59 PID_CURRENT_DEFAULT_GATEWAY  3 / x
        //60 PID_IP_ADDRESS  3 / 3
        //61 PID_SUBNET_MASK  3 / 3
        //62 PID_DEFAULT_GATEWAY  3 / 3
        //64 PID_MAC_ADDRESS  3 / x
        //65 PID_SYSTEM_SETUP_MULTICAST_ADDRESS  3 / x
        //66 PID_ROUTING_MULTICAST_ADDRESS  3 / 3
        //67 PID_TTL  3 / 3
        //68 PID_KNXNETIP_DEVICE_CAPABILITIES  3 / x
        //76 PID_FRIENDLY_NAME  3 / 3
    };
    initializeProperties(sizeof(properties), properties);
}

void IpParameterObject::readProperty(PropertyID propertyId, uint16_t start, uint8_t& count, uint8_t* data)
{
    switch (propertyId)
    {
    case PID_IP_ASSIGNMENT_METHOD:
        data[0] = _ipAssignmentMethod;
        break;
    case PID_IP_CAPABILITIES:
        data[0] = _ipCapabilities;
        break;
    case PID_CURRENT_IP_ADDRESS:
        pushInt(_platform.currentIpAddress(), data);
        break;
    case PID_CURRENT_SUBNET_MASK:
        pushInt(_platform.currentSubnetMask(), data);
        break;
    case PID_CURRENT_DEFAULT_GATEWAY:
        pushInt(_platform.currentDefaultGateway(), data);
        break;
    case PID_IP_ADDRESS:
        pushInt(_ipAddress, data);
        break;
    case PID_SUBNET_MASK:
        pushInt(_subnetMask, data);
        break;
    case PID_DEFAULT_GATEWAY:
        pushInt(_defaultGateway, data);
        break;
    case PID_MAC_ADDRESS:
    {
        uint8_t macAddr[6] = { 0, 0, 0, 0, 0, 0 };
        _platform.macAddress(macAddr);
        pushByteArray(macAddr, 6, data);
        break;
    }
    case PID_SYSTEM_SETUP_MULTICAST_ADDRESS:
        pushInt(DEFAULT_MULTICAST_ADDR, data);
        break;
    case PID_ROUTING_MULTICAST_ADDRESS:
        pushInt(_multicastAddress, data);
        break;
    case PID_TTL:
        data[0] = ttl();
        break;
    case PID_KNXNETIP_DEVICE_CAPABILITIES:
        data[0] = 0x1;
        break;
    case PID_FRIENDLY_NAME:
        for (uint8_t i = start; i < start + count; i++)
            data[i-start] = _friendlyName[i-1];
        break;
    default:
        InterfaceObject::readProperty(propertyId, start, count, data);
        break;
    }
}

void IpParameterObject::writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count)
{
    switch (id)
    {
        case PID_IP_ASSIGNMENT_METHOD:
            _ipAssignmentMethod = data[0];
            break;
        case PID_IP_ADDRESS:
            _ipAddress = getInt(data);
            break;
        case PID_SUBNET_MASK:
            _subnetMask = getInt(data);
            break;
        case PID_DEFAULT_GATEWAY:
            _defaultGateway = getInt(data);
            break;
        case PID_ROUTING_MULTICAST_ADDRESS:
            _multicastAddress = getInt(data);
            break;
        case PID_TTL:
            _ttl = data[0];
            break;
        case PID_FRIENDLY_NAME:
            for (uint8_t i = start; i < start + count; i++)
                _friendlyName[i-1] = data[i - start];
            break;
        default:
            InterfaceObject::writeProperty(id, start, data, count);
            break;
    }
}

uint8_t IpParameterObject::propertySize(PropertyID id)
{
    switch (id)
    {
        case PID_IP_ASSIGNMENT_METHOD:
        case PID_LOAD_STATE_CONTROL:
        case PID_IP_CAPABILITIES:
        case PID_TTL:
        case PID_KNXNETIP_DEVICE_CAPABILITIES:
        case PID_FRIENDLY_NAME:
            return 1;
        case PID_KNX_INDIVIDUAL_ADDRESS:
            return 2;
        case PID_CURRENT_IP_ADDRESS:
        case PID_CURRENT_SUBNET_MASK:
        case PID_CURRENT_DEFAULT_GATEWAY:
        case PID_IP_ADDRESS:
        case PID_SUBNET_MASK:
        case PID_DEFAULT_GATEWAY:
        case PID_SYSTEM_SETUP_MULTICAST_ADDRESS:
        case PID_ROUTING_MULTICAST_ADDRESS:
            return 4;
        case PID_MAC_ADDRESS:
            return 6;
    }
    return InterfaceObject::propertySize(id);
}

uint8_t* IpParameterObject::save(uint8_t* buffer)
{
    buffer = pushWord(_projectInstallationId, buffer);
    buffer = pushByte(_ipAssignmentMethod, buffer);
    buffer = pushByte(_ipCapabilities, buffer);
    buffer = pushInt(_ipAddress, buffer);
    buffer = pushInt(_subnetMask, buffer);
    buffer = pushInt(_defaultGateway, buffer);
    buffer = pushInt(_multicastAddress, buffer);
    buffer = pushByte(_ttl, buffer);
    buffer = pushByteArray((uint8_t*)_friendlyName, 30, buffer);

    return buffer;
}

uint8_t* IpParameterObject::restore(uint8_t* buffer)
{
    buffer = popWord(_projectInstallationId, buffer);
    buffer = popByte(_ipAssignmentMethod, buffer);
    buffer = popByte(_ipCapabilities, buffer);
    buffer = popInt(_ipAddress, buffer);
    buffer = popInt(_subnetMask, buffer);
    buffer = popInt(_defaultGateway, buffer);
    buffer = popInt(_multicastAddress, buffer);
    buffer = popByte(_ttl, buffer);
    buffer = popByteArray((uint8_t*)_friendlyName, 30, buffer);

    return buffer;
}

uint32_t IpParameterObject::multicastAddress() const
{
    if (_multicastAddress == 0)
        return DEFAULT_MULTICAST_ADDR;

    return _multicastAddress;
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_PROJECT_INSTALLATION_ID, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv3 },
};
static uint8_t _propertyDescriptionCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t IpParameterObject::propertyDescriptionCount()
{
    return _propertyDescriptionCount;
}


PropertyDescription* IpParameterObject::propertyDescriptions()
{
    return _propertyDescriptions;
}

uint16_t IpParameterObject::saveSize()
{
    return 51;
}