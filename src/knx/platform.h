#pragma once

#include <stdint.h>
#include <stddef.h>
#include "save_restore.h"

enum NvMemoryType
{
    Eeprom,
    Flash
};

class Platform
{
  public:
    virtual ~Platform() {}

    // ip config
    virtual uint32_t currentIpAddress();
    virtual uint32_t currentSubnetMask();
    virtual uint32_t currentDefaultGateway();
    virtual void macAddress(uint8_t* data);

    // basic stuff
    virtual void restart() = 0;
    virtual void fatalError() = 0;

    //multicast socket
    virtual void setupMultiCast(uint32_t addr, uint16_t port);
    virtual void closeMultiCast();
    virtual bool sendBytesMultiCast(uint8_t* buffer, uint16_t len);
    virtual int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen);

    //unicast socket
    virtual bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len);
    
    //UART
    virtual void setupUart();
    virtual void closeUart();
    virtual int uartAvailable();
    virtual size_t writeUart(const uint8_t data);
    virtual size_t writeUart(const uint8_t* buffer, size_t size);
    virtual int readUart();
    virtual size_t readBytesUart(uint8_t* buffer, size_t length);

    // SPI
    virtual void setupSpi();
    virtual void closeSpi();
    virtual int readWriteSpi(uint8_t *data, size_t len);
#if 0
    // Flash memory
    virtual size_t flashEraseBlockSize(); // in pages
    virtual size_t flashPageSize();       // in bytes
    virtual uint8_t* userFlashStart();   // start of user flash aligned to start of an erase block
    virtual size_t userFlashSizeEraseBlocks(); // in eraseBlocks
    virtual void flashErase(uint16_t eraseBlockNum); //relativ to userFlashStart
    virtual void flashWritePage(uint16_t pageNumber, uint8_t* data); //write a single page to flash (pageNumber relative to userFashStart
#endif
    virtual uint8_t* getEepromBuffer(uint16_t size) = 0;
    virtual void commitToEeprom() = 0;

    NvMemoryType NonVolatileMemoryType();
    void NonVolatileMemoryType(NvMemoryType type);

  protected:
    NvMemoryType _memoryType = Eeprom;
};