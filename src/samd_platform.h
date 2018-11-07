#include "knx/platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_SAMD

#define SerialDBG SerialUSB
#define SerialKNX Serial1

class SamdPlatform : public Platform
{
public:
    SamdPlatform();

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
    virtual void setupUart();
    virtual void closeUart();
    virtual int uartAvailable();
    virtual size_t writeUart(const uint8_t data);
    virtual size_t writeUart(const uint8_t *buffer, size_t size);
    virtual int readUart();
    virtual size_t readBytesUart(uint8_t *buffer, size_t length);

    //memory
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();
private:
    uint32_t _mulitcastAddr;
    uint16_t _mulitcastPort;
};

#endif