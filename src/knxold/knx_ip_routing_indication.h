#pragma once

#include "knx_ip_frame.h"
#include "cemi_frame.h"
#ifdef USE_IP

class KnxIpRoutingIndication : public KnxIpFrame
{
  public:
    KnxIpRoutingIndication(uint8_t* data, uint16_t length);
    KnxIpRoutingIndication(CemiFrame frame);
    CemiFrame& frame();
  private:
    CemiFrame _frame;
};
#endif