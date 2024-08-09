#pragma once

#include "service_families.h"
#if KNX_SERVICE_FAMILY_CORE >= 2
#include "knx_ip_frame.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP
#define REQUESTED_DIBS_MAX 9
class KnxIpSearchRequestExtended : public KnxIpFrame
{
  public:
    KnxIpSearchRequestExtended(uint8_t* data, uint16_t length);
    IpHostProtocolAddressInformation& hpai();
    bool requestedDIB(uint8_t code);
    bool srpByProgMode = false;
    bool srpByMacAddr = false;
    bool srpByService = false;
    bool srpRequestDIBs = false;
    uint8_t *srpMacAddr = nullptr;
    uint8_t *srpServiceFamilies = nullptr;
  private:
    IpHostProtocolAddressInformation _hpai;
    bool requestedDIBs[REQUESTED_DIBS_MAX]; //for now only 1 to 8
};
#endif
#endif