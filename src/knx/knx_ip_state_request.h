#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_cri.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP
class KnxIpStateRequest : public KnxIpFrame
{
  public:
    KnxIpStateRequest(uint8_t* data, uint16_t length);
    IpHostProtocolAddressInformation& hpaiCtrl();
    uint8_t channelId();
  private:
    IpHostProtocolAddressInformation _hpaiCtrl;

};
#endif