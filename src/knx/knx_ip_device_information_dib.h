#pragma once
#include "knx_ip_dib.h"

#ifdef USE_IP
#define LEN_DEVICE_INFORMATION_DIB 54
#define LEN_SERIAL_NUMBER 6
#define LEN_MAC_ADDRESS 6
#define LEN_FRIENDLY_NAME 30

class KnxIpDeviceInformationDIB : public KnxIpDIB
{
  public:
    KnxIpDeviceInformationDIB(uint8_t* data);
    uint8_t medium() const;
    void medium(uint8_t value);
    uint8_t status() const;
    void status(uint8_t value);
    uint16_t individualAddress() const;
    void individualAddress(uint16_t value);
    uint16_t projectInstallationIdentifier() const;
    void projectInstallationIdentifier(uint16_t value);
    const uint8_t* serialNumber() const;
    void serialNumber(const uint8_t* value);
    uint32_t routingMulticastAddress() const;
    void routingMulticastAddress(uint32_t value);
    const uint8_t* macAddress() const;
    void macAddress(const uint8_t* value);
    const uint8_t* friendlyName() const;
    void friendlyName(const uint8_t* value);
};

#endif