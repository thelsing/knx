#include "ip_host_protocol_address_information.h"
#include "bits.h"
#ifdef USE_IP
IpHostProtocolAddressInformation::IpHostProtocolAddressInformation(uint8_t* data)
    : _data(data)
{}


uint8_t IpHostProtocolAddressInformation::length()
{
    return *_data;
}


HostProtocolCode IpHostProtocolAddressInformation::code()
{
    return (HostProtocolCode)_data[1];
}


uint32_t IpHostProtocolAddressInformation::ipAddress()
{
    return getInt(_data + 2);
}


uint16_t IpHostProtocolAddressInformation::ipPortNumber()
{
    return getWord(_data + 6);
}
#endif