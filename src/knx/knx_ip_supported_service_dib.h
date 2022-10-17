#pragma once
#include "knx_ip_dib.h"

#ifdef USE_IP
enum ServiceFamily : uint8_t
{
    Core = 2,
    DeviceManagement = 3,
    Tunnelling = 4,
    Routing = 5,
    RemoteLogging = 6,
    RemoteConfigDiag = 7,
    ObjectServer = 8
};

class KnxIpSupportedServiceDIB : public KnxIpDIB
{
  public:
    KnxIpSupportedServiceDIB(uint8_t* data);
    uint8_t serviceVersion(ServiceFamily family);
    void serviceVersion(ServiceFamily family, uint8_t version);
};
#endif