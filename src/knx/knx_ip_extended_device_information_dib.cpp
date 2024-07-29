#include "knx_ip_extended_device_information_dib.h"
#include "bits.h"

#ifdef USE_IP
KnxIpExtendedDeviceInformationDIB::KnxIpExtendedDeviceInformationDIB(uint8_t* data) : KnxIpDIB(data)
{}

uint8_t KnxIpExtendedDeviceInformationDIB::status() const
{
    return _data[2];
}


void KnxIpExtendedDeviceInformationDIB::status(uint8_t value)
{
    _data[2] = value;
}


uint16_t KnxIpExtendedDeviceInformationDIB::localMaxApdu() const
{
    return getWord(_data + 4);
}


void KnxIpExtendedDeviceInformationDIB::localMaxApdu(uint16_t value)
{
    pushWord(value, _data + 4);
}


uint16_t KnxIpExtendedDeviceInformationDIB::deviceDescriptor() const
{
    return getWord(_data + 6);
}


void KnxIpExtendedDeviceInformationDIB::deviceDescriptor(uint16_t value)
{
    pushWord(value, _data + 6);
}
#endif