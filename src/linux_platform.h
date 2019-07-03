#pragma once

#ifdef __linux__

#include <string>
#include "knx/platform.h"

class LinuxPlatform: public Platform
{
    using Platform::_memoryReference;
    using Platform::memoryReference;
public:
    LinuxPlatform(int argc, char** argv);
    virtual ~LinuxPlatform();

    std::string flashFilePath();
    void flashFilePath(const std::string path);

    // ip stuff
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // basic stuff
    uint32_t millis() override;
    void mdelay(uint32_t millis) override;
    void restart() override;
    void fatalError() override;

    //multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytes(uint8_t* buffer, uint16_t len) override;
    int readBytes(uint8_t* buffer, uint16_t maxLen) override;
    
    //uart
    void setupUart() override;
    void closeUart() override;
    int uartAvailable() override;
    size_t writeUart(const uint8_t data) override;
    size_t writeUart(const uint8_t *buffer, size_t size) override;
    int readUart() override;
    size_t readBytesUart(uint8_t *buffer, size_t length) override;

    //memory
    uint8_t* getEepromBuffer(uint16_t size) override;
    void commitToEeprom() override;
    uint8_t* allocMemory(size_t size) override;
    void freeMemory(uint8_t* ptr) override;
    void cmdlineArgs(int argc, char** argv);

  private:
    uint32_t _multicastAddr;
    uint16_t _port;
    int _socketFd = -1;
    void doMemoryMapping();
    uint8_t* _mappedFile;
    int _fd = -1;
    uint8_t* _currentMaxMem = 0;
    std::string _flashFilePath = "flash.bin";
    char** _args;
};

#endif