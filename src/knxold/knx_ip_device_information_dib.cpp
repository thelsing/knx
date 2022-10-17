#include "knx_ip_device_information_dib.h"
#include "bits.h"

#ifdef USE_IP
KnxIpDeviceInformationDIB::KnxIpDeviceInformationDIB(uint8_t* data) : KnxIpDIB(data)
{}

uint8_t KnxIpDeviceInformationDIB::medium() const
{
    return _data[2];
}


void KnxIpDeviceInformationDIB::medium(uint8_t value)
{
    _data[2] = value;
}


uint8_t KnxIpDeviceInformationDIB::status() const
{
    return _data[3];
}


void KnxIpDeviceInformationDIB::status(uint8_t value)
{
    _data[3] = value;
}


uint16_t KnxIpDeviceInformationDIB::individualAddress() const
{
    return getWord(_data + 4);
}


void KnxIpDeviceInformationDIB::individualAddress(uint16_t value)
{
    pushWord(value, _data + 4);
}


uint16_t KnxIpDeviceInformationDIB::projectInstallationIdentifier() const
{
    return getWord(_data + 6);
}


void KnxIpDeviceInformationDIB::projectInstallationIdentifier(uint16_t value)
{
    pushWord(value, _data + 6);
}


const uint8_t* KnxIpDeviceInformationDIB::serialNumber() const
{
    return _data + 8;
}


void KnxIpDeviceInformationDIB::serialNumber(const uint8_t* value)
{
    pushByteArray(value, LEN_SERIAL_NUMBER, _data + 8);
}


uint32_t KnxIpDeviceInformationDIB::routingMulticastAddress() const
{
    return getInt(_data + 14);
}


void KnxIpDeviceInformationDIB::routingMulticastAddress(uint32_t value)
{
    pushInt(value, _data + 14);
}


const uint8_t* KnxIpDeviceInformationDIB::macAddress() const
{
    return _data + 18;
}


void KnxIpDeviceInformationDIB::macAddress(const uint8_t* value)
{
    pushByteArray(value, LEN_MAC_ADDRESS, _data + 18);
}


const uint8_t* KnxIpDeviceInformationDIB::friendlyName() const
{
    return _data + 24;
}


void KnxIpDeviceInformationDIB::friendlyName(const uint8_t* value)
{
    pushByteArray(value, LEN_FRIENDLY_NAME, _data + 24);
}
#endif