#pragma once

#include "config.h"
#ifdef USE_RF

#include <stdint.h>

#include "rf_physical_layer.h"

#define RX_PACKET_TIMEOUT        20   // Wait 20ms for packet reception to complete

// Calculate the real packet size out of the L-field of FT3 frame data. See KNX-RF spec. 3.2.5 Data Link Layer frame format
#define PACKET_SIZE(lField) ((((lField - 10 /*size of first pkt*/))/16 + 2 /*CRC in first pkt */) * 2 /*to bytes*/ +lField + 1 /*size of len byte*/)

// loop states
#define RX_START 0
#define RX_ACTIVE 1
#define RX_END 2
#define TX_START 3
#define TX_ACTIVE 4
#define TX_END 5

class RfDataLinkLayer;

class RfPhysicalLayerCC1310 : public RfPhysicalLayer
{
  public:
    RfPhysicalLayerCC1310(RfDataLinkLayer& rfDataLinkLayer, Platform& platform);

    virtual bool InitChip() override;
    virtual void stopChip() override;
    virtual void loop() override;

    void setOutputPowerLevel(int8_t dBm);

  private:
    uint16_t pktLen {0};
    uint8_t *sendBuffer {0};
    uint16_t sendBufferLength {0};

    uint8_t _loopState = RX_START;
};

#endif
