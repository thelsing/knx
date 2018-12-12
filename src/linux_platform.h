#pragma once

#ifdef __linux__

#include "knx/platform.h"

class LinuxPlatform: public Platform
{
    using Platform::_memoryReference;
public:
    LinuxPlatform();

    // ip stuff
    uint32_t currentIpAddress();
    uint32_t currentSubnetMask();
    uint32_t currentDefaultGateway();
    void macAddress(uint8_t* addr);

    // basic stuff
    uint32_t millis();
    void mdelay(uint32_t millis);
    void restart();
    void fatalError();

    //multicast
    void setupMultiCast(uint32_t addr, uint16_t port);
    void closeMultiCast();
    bool sendBytes(uint8_t* buffer, uint16_t len);
    int readBytes(uint8_t* buffer, uint16_t maxLen);
    
    //uart
    void setupUart();
    void closeUart();
    int uartAvailable();
    size_t writeUart(const uint8_t data);
    size_t writeUart(const uint8_t *buffer, size_t size);
    int readUart();
    size_t readBytesUart(uint8_t *buffer, size_t length);

    //memory
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();
    uint8_t* allocMemory(size_t size);
    void freeMemory(uint8_t* ptr);
private:
    uint32_t _multicastAddr;
    uint16_t _port;
    int _socketFd = -1;
    void doMemoryMapping();
    uint8_t* _mappedFile;
    int _fd;
    uint8_t* _currentMaxMem = 0;
};

#endif