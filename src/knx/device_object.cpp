#include <cstring>
#include "device_object.h"
#include "bits.h"

void DeviceObject::readProperty(PropertyID propertyId, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (propertyId)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_DEVICE, data);
            break;
        case PID_SERIAL_NUMBER:
            pushByteArray((uint8_t*)_knxSerialNumber, 6, data);
            break;
        case PID_MANUFACTURER_ID:
            pushByteArray(&_knxSerialNumber[0], 2, data);
            break;
        case PID_DEVICE_CONTROL:
            *data = _deviceControl;
            break;
        case PID_ORDER_INFO:
            pushByteArray((uint8_t*)_orderNumber, 10, data);
            break;
        case PID_HARDWARE_TYPE:
            pushByteArray((uint8_t*)_hardwareType, LEN_HARDWARE_TYPE, data);
            break;
        case PID_VERSION:
            pushWord(_version, data);
            break;
        case PID_ROUTING_COUNT:
            *data = _routingCount;
            break;
        case PID_PROG_MODE:
            *data = _prgMode;
            break;
        case PID_MAX_APDU_LENGTH:
            pushWord(_maxApduLength, data);
            break;
        case PID_SUBNET_ADDR:
            *data = ((_ownAddress >> 8) & 0xff);
            break;
        case PID_DEVICE_ADDR:
            *data = (_ownAddress & 0xff);
            break;
        case PID_IO_LIST:
        {
            for (uint32_t i = start; i < (_ifObjs[0] + 1) && i < count; i++)
                pushInt(_ifObjs[i], data);
            break;
        }
        case PID_DEVICE_DESCRIPTOR:
            pushWord(_maskVersion, data);
            break;
        case PID_RF_DOMAIN_ADDRESS_CEMI_SERVER:
            pushByteArray((uint8_t*)_rfDomainAddress, 6, data);
            break;
        default:
            count = 0;
    }
}

void DeviceObject::writeProperty(PropertyID id, uint32_t start, uint8_t* data, uint32_t& count)
{
    switch (id)
    {
        case PID_DEVICE_CONTROL:
            _deviceControl = data[0];
            break;
        case PID_ROUTING_COUNT:
            _routingCount = data[0];
            break;
        case PID_PROG_MODE:
            _prgMode = data[0];
            break;
        case PID_RF_DOMAIN_ADDRESS_CEMI_SERVER:
            memcpy(_rfDomainAddress, data, propertySize(PID_RF_DOMAIN_ADDRESS_CEMI_SERVER));
            break;
        case PID_SUBNET_ADDR:
            _ownAddress = (data[0] << 8) | (_ownAddress & 0xff);
            break;
        case PID_DEVICE_ADDR:
            _ownAddress = data[0] | (_ownAddress & 0xff00);
            break;
        default:
            count = 0;
            break;
    }
}

uint8_t DeviceObject::propertySize(PropertyID id)
{
    switch (id)
    {
    case PID_DEVICE_CONTROL:
    case PID_ROUTING_COUNT:
    case PID_PROG_MODE:
    case PID_SUBNET_ADDR:
    case PID_DEVICE_ADDR:
        return 1;
    case PID_OBJECT_TYPE:
    case PID_MANUFACTURER_ID:
    case PID_VERSION:
    case PID_DEVICE_DESCRIPTOR:
    case PID_MAX_APDU_LENGTH:
        return 2;
    case PID_IO_LIST:
        return 4;
    case PID_SERIAL_NUMBER:
    case PID_HARDWARE_TYPE:
    case PID_RF_DOMAIN_ADDRESS_CEMI_SERVER:
        return 6;
    case PID_ORDER_INFO:
        return 10;
    }
    return 0;
}

uint8_t* DeviceObject::save(uint8_t* buffer)
{
    buffer = pushByte(_deviceControl, buffer);
    buffer = pushByte(_routingCount, buffer);
    buffer = pushWord(_ownAddress, buffer);
    buffer = pushByteArray((uint8_t*)_rfDomainAddress, 6, buffer);
    return buffer;
}

uint8_t* DeviceObject::restore(uint8_t* buffer)
{
    buffer = popByte(_deviceControl, buffer);
    buffer = popByte(_routingCount, buffer);
    buffer = popWord(_ownAddress, buffer);
    buffer = popByteArray((uint8_t*)_rfDomainAddress, 6, buffer);
    _prgMode = 0;
    return buffer;
}

uint16_t DeviceObject::induvidualAddress()
{
    return _ownAddress;
}

void DeviceObject::induvidualAddress(uint16_t value)
{
    _ownAddress = value;
}

#define USER_STOPPED  0x1
#define OWN_ADDR_DUPL 0x2
#define VERIFY_MODE   0x4
#define SAFE_STATE    0x8


bool DeviceObject::userStopped()
{
    return (_deviceControl & USER_STOPPED) > 0;
}

void DeviceObject::userStopped(bool value)
{
    if (value)
        _deviceControl |= USER_STOPPED;
    else
        _deviceControl &= ~USER_STOPPED;
}

bool DeviceObject::induvidualAddressDuplication()
{
    return (_deviceControl & OWN_ADDR_DUPL) > 0;
}

void DeviceObject::induvidualAddressDuplication(bool value)
{
    if (value)
        _deviceControl |= OWN_ADDR_DUPL;
    else
        _deviceControl &= ~OWN_ADDR_DUPL;
}

bool DeviceObject::verifyMode()
{
    return (_deviceControl & VERIFY_MODE) > 0;
}

void DeviceObject::verifyMode(bool value)
{
    if (value)
        _deviceControl |= VERIFY_MODE;
    else
        _deviceControl &= ~VERIFY_MODE;
}

bool DeviceObject::safeState()
{
    return (_deviceControl & SAFE_STATE) > 0;
}

void DeviceObject::safeState(bool value)
{
    if (value)
        _deviceControl |= SAFE_STATE;
    else
        _deviceControl &= ~SAFE_STATE;
}

bool DeviceObject::progMode()
{
    return _prgMode == 1;
}

void DeviceObject::progMode(bool value)
{
    if (value)
        _prgMode = 1;
    else
        _prgMode = 0;
}

uint16_t DeviceObject::manufacturerId()
{
    uint16_t manufacturerId;
    popWord(manufacturerId, &_knxSerialNumber[0]);
    return manufacturerId;
}

void DeviceObject::manufacturerId(uint16_t value)
{
   pushWord(value, &_knxSerialNumber[0]);
}

uint32_t DeviceObject::bauNumber()
{
    uint32_t bauNumber;
    popInt(bauNumber, &_knxSerialNumber[2]);
    return bauNumber;
}

void DeviceObject::bauNumber(uint32_t value)
{
   pushInt(value, &_knxSerialNumber[2]);
}

const uint8_t* DeviceObject::knxSerialNumber()
{
    return _knxSerialNumber;
}

void DeviceObject::knxSerialNumber(const uint8_t* value)
{
    pushByteArray(value, 6, _knxSerialNumber);
}

const char* DeviceObject::orderNumber()
{
    return _orderNumber;
}

void DeviceObject::orderNumber(const char* value)
{
    strncpy(_orderNumber, value, 10);
}

const uint8_t* DeviceObject::hardwareType()
{
    return _hardwareType;
}

void DeviceObject::hardwareType(const uint8_t* value)
{
    pushByteArray(value, 6, _hardwareType);
}

uint16_t DeviceObject::version()
{
    return _version;
}

void DeviceObject::version(uint16_t value)
{
    _version = value;
}

uint16_t DeviceObject::maskVersion()
{
    return _maskVersion;
}

void DeviceObject::maskVersion(uint16_t value)
{
    _maskVersion = value;
}

void DeviceObject::maxApduLength(uint16_t value)
{
    _maxApduLength = value;
}

uint16_t DeviceObject::maxApduLength()
{
    return _maxApduLength;
}

const uint32_t* DeviceObject::ifObj()
{
    return _ifObjs;
}

void DeviceObject::ifObj(const uint32_t* value)
{
    _ifObjs = value;
}

uint8_t* DeviceObject::rfDomainAddress()
{
    return _rfDomainAddress;
}

void DeviceObject::rfDomainAddress(uint8_t* value)
{
    pushByteArray(value, 6, _rfDomainAddress);
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_SERIAL_NUMBER, false, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0 }
};
static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t DeviceObject::propertyCount()
{
    return _propertyCount;
}


PropertyDescription* DeviceObject::propertyDescriptions()
{
    return _propertyDescriptions;
}

uint16_t DeviceObject::saveSize()
{
    return 4;
}