#pragma once

#include "knx_ip_frame.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP
class KnxIpDisconnectRequest : public KnxIpFrame
{
  public:
    KnxIpDisconnectRequest(uint8_t* data, uint16_t length);
    KnxIpDisconnectRequest();
    IpHostProtocolAddressInformation& hpaiCtrl();
    uint8_t channelId();
    void channelId(uint8_t channelId);
  private:
    IpHostProtocolAddressInformation _hpaiCtrl;
};
#endif