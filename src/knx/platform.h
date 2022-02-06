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

    virtual uint8_t* getNonVolatileMemoryStart();
    virtual size_t getNonVolatileMemorySize();
    virtual void commitNonVolatileMemory();
    // address is relative to start of nonvolatile memory
    virtual uint32_t writeNonVolatileMemory(uint32_t relativeAddress, uint8_t* buffer, size_t size);

    NvMemoryType NonVolatileMemoryType();
    void NonVolatileMemoryType(NvMemoryType type);

  protected:
        // Flash memory
    
    // size of one EraseBlock in pages
    virtual size_t flashEraseBlockSize();
    // size of one flash page in bytes
    virtual size_t flashPageSize();
    // start of user flash aligned to start of an erase block
    virtual uint8_t* userFlashStart();
    virtual size_t userFlashSizeEraseBlocks(); // in eraseBlocks
    virtual void flashErase(uint16_t eraseBlockNum); //relativ to userFlashStart
    virtual void flashWritePage(uint16_t pageNumber, uint8_t* data); //write a single page to flash (pageNumber relative to userFashStart
    
    NvMemoryType _memoryType = Eeprom;

  private:
    void loadEraseblockContaining(uint32_t relativeAddress);
    uint32_t bufferedEraseBlockStart();
    uint32_t bufferedEraseBlockEnd();
    int32_t getEraseBlockNumberOf(uint32_t relativeAddress);
    // writes _eraseblockBuffer to flash
    void writeBufferedEraseBlock();
    // copies a EraseBlock into the _eraseblockBuffer
    void bufferEraseBlock(uint32_t eraseBlockNumber);

    // in theory we would have to use this buffer for memory reads too,
    // but because ets always restarts the device after programming it
    // we can ignore this issue
    uint8_t* _eraseblockBuffer = nullptr;
    int32_t _bufferedEraseblockNumber = -1;
    bool _bufferedEraseblockDirty = false;
};