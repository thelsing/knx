#pragma once

#include "config.h"
#ifdef USE_RF

#include <stdint.h>

#include "platform.h"

// Calculate the real packet size out of the L-field of FT3 frame data. See KNX-RF spec. 3.2.5 Data Link Layer frame format
#define PACKET_SIZE(lField) ((((lField - 10 /*size of first pkt*/))/16 + 2 /*CRC in first pkt */) * 2 /*to bytes*/ +lField + 1 /*size of len byte*/)

class RfDataLinkLayer;

class RfPhysicalLayer
{
  public:
    RfPhysicalLayer(RfDataLinkLayer& rfDataLinkLayer, Platform& platform) 
    : _rfDataLinkLayer(rfDataLinkLayer), _platform(platform) {}

    virtual bool InitChip() = 0;
    virtual void stopChip() = 0;
    virtual void loop() = 0;

  protected:
    RfDataLinkLayer& _rfDataLinkLayer;
    Platform& _platform;
};

#endif
