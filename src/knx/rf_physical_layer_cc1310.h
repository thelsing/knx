#pragma once

#ifdef DeviceFamily_CC13X0

#include "config.h"
#ifdef USE_RF

#include <stdint.h>

#include "rf_physical_layer.h"

#define RX_PACKET_TIMEOUT        20   // Wait 20ms for packet reception to complete

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
    uint8_t _loopState = RX_START;
};

#endif // USE_RF

#endif // DeviceFamily_CC13X0