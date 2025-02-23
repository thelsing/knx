#include "ip_host_protocol_address_information.h"
#include "bits.h"
#ifdef USE_IP
IpHostProtocolAddressInformation::IpHostProtocolAddressInformation(uint8_t* data)
    : _data(data)
{}


uint8_t IpHostProtocolAddressInformation::length() const
{
    return *_data;
}

void IpHostProtocolAddressInformation::length(uint8_t value)
{
    *_data = value;
}

HostProtocolCode IpHostProtocolAddressInformation::code() const
{
    return (HostProtocolCode)_data[1];
}

void IpHostProtocolAddressInformation::code(HostProtocolCode value)
{
    _data[1] = value;
}

uint32_t IpHostProtocolAddressInformation::ipAddress() const
{
    return getInt(_data + 2);
}

void IpHostProtocolAddressInformation::ipAddress(uint32_t value)
{
    pushInt(value, _data + 2);
}

uint16_t IpHostProtocolAddressInformation::ipPortNumber() const
{
    return getWord(_data + 6);
}

void IpHostProtocolAddressInformation::ipPortNumber(uint16_t value)
{
    pushWord(value, _data + 6);
}
#endif
