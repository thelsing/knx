#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_cri.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP
class KnxIpDescriptionRequest : public KnxIpFrame
{
  public:
    KnxIpDescriptionRequest(uint8_t* data, uint16_t length);
    IpHostProtocolAddressInformation& hpaiCtrl();
  private:
    IpHostProtocolAddressInformation _hpaiCtrl;
};
#endif