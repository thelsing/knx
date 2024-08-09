#pragma once
#include "knx_ip_dib.h"

#ifdef USE_IP
#define LEN_EXTENDED_DEVICE_INFORMATION_DIB 8

class KnxIpExtendedDeviceInformationDIB : public KnxIpDIB
{
  public:
    KnxIpExtendedDeviceInformationDIB(uint8_t* data);
    uint8_t status() const;
    void status(uint8_t value);
    uint16_t localMaxApdu() const;
    void localMaxApdu(uint16_t value);
    uint16_t deviceDescriptor() const;
    void deviceDescriptor(uint16_t value);
};

#endif