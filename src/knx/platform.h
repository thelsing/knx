#pragma once

#include <stdint.h>
#include <stddef.h>
#include "save_restore.h"

class Platform
{
  public:
    // ip config
    virtual uint32_t currentIpAddress() = 0;
    virtual uint32_t currentSubnetMask() = 0;
    virtual uint32_t currentDefaultGateway() = 0;
    virtual void macAddress(uint8_t* data) = 0;

    // basic stuff
    virtual void restart() = 0;
    virtual void fatalError() = 0;

    //multicast socket
    virtual void setupMultiCast(uint32_t addr, uint16_t port) = 0;
    virtual void closeMultiCast() = 0;
    virtual bool sendBytes(uint8_t* buffer, uint16_t len) = 0;
    virtual int readBytes(uint8_t* buffer, uint16_t maxLen) = 0;
    
    //UART
    virtual void setupUart() = 0;
    virtual void closeUart() = 0;
    virtual int uartAvailable() = 0;
    virtual size_t writeUart(const uint8_t data) = 0;
    virtual size_t writeUart(const uint8_t* buffer, size_t size) = 0;
    virtual int readUart() = 0;
    virtual size_t readBytesUart(uint8_t* buffer, size_t length) = 0;

    // SPI
    virtual void setupSpi() = 0;
    virtual void closeSpi() = 0;
    virtual int readWriteSpi (uint8_t *data, size_t len) = 0;
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
};