#pragma once
#include "knx_ip_dib.h"
#include "bits.h"

#ifdef USE_IP
#define LEN_IP_CONFIG_DIB 16
#define LEN_IP_CURRENT_CONFIG_DIB 20

class KnxIpConfigDIB : public KnxIpDIB
{
  public:
    KnxIpConfigDIB(uint8_t* data, bool isCurrent = false);
    uint32_t address();
    void address(uint32_t addr);
    uint32_t subnet();
    void subnet(uint32_t addr);
    uint32_t gateway();
    void gateway(uint32_t addr);
    uint32_t dhcp();
    void dhcp(uint32_t addr);
    uint8_t info1();
    void info1(uint8_t addr);
    uint8_t info2();
    void info2(uint8_t addr);
  private:
    bool _isCurrent = false;
};
#endif