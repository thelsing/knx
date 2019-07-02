#pragma once

#include <stdint.h>
#include <stddef.h>
#include "save_restore.h"

class Platform
{
  public:
    Platform();
    virtual uint32_t currentIpAddress() = 0;
    virtual uint32_t currentSubnetMask() = 0;
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

    virtual void setupUart() = 0;
    virtual void closeUart() = 0;
    virtual int uartAvailable() = 0;
    virtual size_t writeUart(const uint8_t data) = 0;
    virtual size_t writeUart(const uint8_t* buffer, size_t size) = 0;
    virtual int readUart() = 0;
    virtual size_t readBytesUart(uint8_t* buffer, size_t length) = 0;

    virtual uint8_t* getEepromBuffer(uint16_t size) = 0;
    virtual void commitToEeprom() = 0;

    virtual uint8_t* memoryReference();
    virtual uint8_t* allocMemory(size_t size);
    virtual void freeMemory(uint8_t* ptr);

  protected:
    uint8_t* _memoryReference = 0;
};