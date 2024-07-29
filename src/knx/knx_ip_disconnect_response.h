#pragma once

#include "knx_ip_frame.h"
#ifdef USE_IP

class KnxIpDisconnectResponse : public KnxIpFrame
{
  public:
    KnxIpDisconnectResponse(uint8_t channel, uint8_t status);
  private:
};

#endif