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
    virtual void restart() final;
    virtual void fatalError() final;

    virtual uint8_t* getEepromBuffer(uint16_t size) final;
    virtual void commitToEeprom() final;
};

#endif //DeviceFamily_CC13X0
