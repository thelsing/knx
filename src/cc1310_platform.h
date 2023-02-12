#pragma once

#ifdef DeviceFamily_CC13X0

#include <ti/drivers/NVS.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/dpl/ClockP.h>

#include "knx/platform.h"

class CC1310Platform : public Platform
{
  public:
    CC1310Platform();
    virtual ~CC1310Platform();

    void init();

    // basic stuff
    void restart() final;
    void fatalError() final;

    uint8_t* getEepromBuffer(uint32_t size) final;
    void commitToEeprom() final;
};

#endif //DeviceFamily_CC13X0
