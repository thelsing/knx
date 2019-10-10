#pragma once

#include <stdint.h>
#include <stddef.h>
#include "save_restore.h"

typedef enum{
    notDefined,
    internalRam,
    internalFlash,
    external
}NVMemory_t;

class Platform
{
  public:
    Platform();
    virtual uint32_t currentIpAddress() = 0;
    virtual uint32_t currentSubnetMask() = 0;
    virtual uint32_t currentDefaultGateway() = 0;
    virtual void macAddress(uint8_t* data) = 0;

    virtual void restart() = 0;
    virtual void fatalError() = 0;

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

    virtual bool writeNVMemory(uintptr_t addr,uint8_t data) = 0;
    virtual uint8_t readNVMemory(uintptr_t addr) = 0;
    virtual uintptr_t allocNVMemory(size_t size,uint32_t ID) = 0;
    virtual uintptr_t reloadNVMemory(uint32_t ID) = 0;
    virtual void finishNVMemory() = 0;
    virtual void freeNVMemory(uint32_t ID) = 0;

    virtual uint8_t* memoryReference();
    virtual uint8_t* allocMemory(size_t size);
    virtual void freeMemory(uint8_t* ptr);
    NVMemory_t NVMemoryType(){return _NVMemoryType;}
  protected:
    uint8_t* _memoryReference = 0;
    NVMemory_t _NVMemoryType = notDefined;
};
