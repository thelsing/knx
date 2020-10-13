#pragma once

#include <ti/drivers/NVS.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/dpl/ClockP.h>

#include "knx/platform.h"

#define EEPROM_EMULATION_SIZE 2048

class CC1310Platform : public Platform
{
  public:
    CC1310Platform();
    virtual ~CC1310Platform();

    void earlyInit();

    // basic stuff
    virtual void restart() final;
    virtual void fatalError() final;

    virtual uint8_t* getEepromBuffer(uint16_t size) final;
    virtual void commitToEeprom() final;

    static uint32_t millis();

    private:
    void msClockInit();
    void InitUART();
    void InitNVS();

    static void clk0Fxn(uintptr_t arg0);
    static volatile uint32_t msCounter;

    uint8_t _serialNumber[6];
    uint8_t _NVS_buffer[EEPROM_EMULATION_SIZE];

    ClockP_Struct clk0Struct;
    ClockP_Handle clk0Handle;
    UART_Handle uart;
    NVS_Handle nvsHandle;
};
