#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_ch.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP
class KnxIpConfigRequest : public KnxIpFrame
{
  public:
    KnxIpConfigRequest(uint8_t* data, uint16_t length);
    CemiFrame& frame();
    KnxIpCH& connectionHeader();
  private:
    CemiFrame _frame;
    KnxIpCH _ch;
};
#endif