#pragma once

#include <cstdint>
#include "config.h"

enum HostProtocolCode : uint8_t
{
    IPV4_UDP = 1,
    IPV4_TCP = 2
};

#ifdef USE_IP

#define LEN_IPHPAI 8
#define LEN_CRD 4

class IpHostProtocolAddressInformation
{
  public:
    IpHostProtocolAddressInformation(uint8_t* data);
    uint8_t length() const;
    void length(uint8_t value);
    HostProtocolCode code() const;
    void code(HostProtocolCode value);
    uint32_t ipAddress() const;
    void ipAddress(uint32_t value);
    uint16_t ipPortNumber() const;
    void ipPortNumber(uint16_t value);

  private:
    uint8_t* _data;
};
#endif