#pragma once

#include <stdint.h>
#include "save_restore.h"

class Platform
{
public:
    virtual uint32_t currentIpAddress() = 0;
    virtual uint32_t currentSubnetMask()  = 0;
    virtual uint32_t currentDefaultGateway() = 0;
    virtual void macAddress(uint8_t* data) = 0;

    virtual uint32_t millis() = 0;
    virtual void restart() = 0;
    virtual void fatalError() = 0;
    virtual void mdelay(uint32_t millis) = 0;
    
    virtual void setupMultiCast(uint32_t addr, uint16_t port) = 0;
    virtual void closeMultiCast() = 0;
    virtual bool sendBytes(uint8_t* buffer, uint16_t len) = 0;
    virtual int readBytes(uint8_t* buffer, uint16_t maxLen) = 0;
    
    virtual uint8_t* getEepromBuffer(uint16_t size) = 0;
    virtual void commitToEeprom() = 0;
};