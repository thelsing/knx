#include "ip_parameter_object.h"
#include "device_object.h"
#include "platform.h"
#include "bits.h"

//224.0.23.12
#define DEFAULT_MULTICAST_ADDR 0xE000170C

IpParameterObject::IpParameterObject(DeviceObject& deviceObject, Platform& platform): _deviceObject(deviceObject),
    _platform(platform)
{}

void IpParameterObject::readProperty(PropertyID propertyId, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (propertyId)
    {
    case PID_LOAD_STATE_CONTROL:
        data[0] = _state;
        break;
    case PID_OBJECT_TYPE:
        pushWord(OT_IP_PARAMETER, data);
        break;
    case PID_PROJECT_INSTALLATION_ID:
        pushWord(_projectInstallationId, data);
        break;
    case PID_KNX_INDIVIDUAL_ADDRESS:
        pushWord(_deviceObject.induvidualAddress(), data);
        break;
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
        count = 0;
        break;
    }
}

void IpParameterObject::writeProperty(PropertyID id, uint32_t start, uint8_t* data, uint32_t& count)
{
    switch (id)
    {
        case PID_LOAD_STATE_CONTROL:
            loadEvent(data);
            break;
        case PID_PROJECT_INSTALLATION_ID:
            _projectInstallationId = getWord(data);
            break;
        case PID_KNX_INDIVIDUAL_ADDRESS:
            _deviceObject.induvidualAddress(getWord(data));
            break;
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
            count = 0;
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
        case PID_OBJECT_TYPE:
        case PID_PROJECT_INSTALLATION_ID:
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
    return 0;
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

void IpParameterObject::loadEvent(uint8_t* data)
{
    switch (_state)
    {
    case LS_UNLOADED:
        loadEventUnloaded(data);
        break;
    case LS_LOADING:
        loadEventLoading(data);
        break;
    case LS_LOADED:
        loadEventLoaded(data);
        break;
    case LS_ERROR:
        loadEventError(data);
        break;
    }
}

void IpParameterObject::loadState(LoadState newState)
{
    if (newState == _state)
        return;
    //beforeStateChange(newState);
    _state = newState;
}

void IpParameterObject::loadEventUnloaded(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
    case LE_NOOP:
    case LE_LOAD_COMPLETED:
    case LE_ADDITIONAL_LOAD_CONTROLS:
    case LE_UNLOAD:
        break;
    case LE_START_LOADING:
        loadState(LS_LOADING);
        break;
    default:
        loadState(LS_ERROR);
        _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void IpParameterObject::loadEventLoading(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
    case LE_NOOP:
    case LE_START_LOADING:
        break;
    case LE_LOAD_COMPLETED:
        loadState(LS_LOADED);
        break;
    case LE_UNLOAD:
        loadState(LS_UNLOADED);
        break;
    case LE_ADDITIONAL_LOAD_CONTROLS:
        additionalLoadControls(data);
        break;
    default:
        loadState(LS_ERROR);
        _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void IpParameterObject::loadEventLoaded(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
    case LE_NOOP:
    case LE_LOAD_COMPLETED:
        break;
    case LE_START_LOADING:
        loadState(LS_LOADING);
        break;
    case LE_UNLOAD:
        loadState(LS_UNLOADED);
        break;
    case LE_ADDITIONAL_LOAD_CONTROLS:
        loadState(LS_ERROR);
        _errorCode = E_INVALID_OPCODE;
        break;
    default:
        loadState(LS_ERROR);
        _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void IpParameterObject::loadEventError(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
    case LE_NOOP:
    case LE_LOAD_COMPLETED:
    case LE_ADDITIONAL_LOAD_CONTROLS:
    case LE_START_LOADING:
        break;
    case LE_UNLOAD:
        loadState(LS_UNLOADED);
        break;
    default:
        loadState(LS_ERROR);
        _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void IpParameterObject::additionalLoadControls(uint8_t* data)
{
    loadState(LS_ERROR);
    _errorCode = E_INVALID_OPCODE;
    return;
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_PROJECT_INSTALLATION_ID, true, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv3 },
};
static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t IpParameterObject::propertyCount()
{
    return _propertyCount;
}


PropertyDescription* IpParameterObject::propertyDescriptions()
{
    return _propertyDescriptions;
}