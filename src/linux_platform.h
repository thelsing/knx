#pragma once

#ifdef __linux__

#include <string>
#include "knx/platform.h"

extern int gpio_direction(int pin, int dir);
extern int gpio_read(int pin);
extern int gpio_write(int pin, int value);
extern int gpio_export(int pin);
extern int gpio_unexport(int pin);

class LinuxPlatform: public Platform
{
public:
    LinuxPlatform();
    virtual ~LinuxPlatform();

    void cmdLineArgs(int argc, char** argv);

    std::string flashFilePath();
    void flashFilePath(const std::string path);

    // ip stuff
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // basic stuff
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

    //spi
    void setupSpi() override;
    void closeSpi() override;
    int readWriteSpi (uint8_t *data, size_t len) override;

    //memory
    uint8_t* getEepromBuffer(uint16_t size) override;
    void commitToEeprom() override;
    void cmdlineArgs(int argc, char** argv);

  private:
    uint32_t _multicastAddr = -1;
    uint16_t _port = -1;
    int _socketFd = -1;
    void doMemoryMapping();
    uint8_t* _mappedFile = 0;
    int _fd = -1;
    int _spiFd = -1;
    uint8_t* _currentMaxMem = 0;
    std::string _flashFilePath = "flash.bin";
    char** _args = 0;
};

#endif