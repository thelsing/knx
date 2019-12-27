#pragma once

#include <cstdint>
#include "config.h"

enum HostProtocolCode : uint8_t
{
    IPV4_UDP = 1,
    IPV4_TCP = 2
};

#ifdef USE_IP
class IpHostProtocolAddressInformation
{
  public:
    IpHostProtocolAddressInformation(uint8_t* data);
    uint8_t length();
    HostProtocolCode code();
    uint32_t ipAddress();
    uint16_t ipPortNumber();
  private:
    uint8_t* _data;
};
#endif