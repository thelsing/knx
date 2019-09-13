#include "knx/platform.h"

#include "Arduino.h"

extern Stream& _serialDBG;

class ArduinoPlatform : public Platform
{
  public:
    ArduinoPlatform(HardwareSerial* knxSerial);

    // ip stuff
    uint32_t currentIpAddress();
    uint32_t currentSubnetMask();
    uint32_t currentDefaultGateway();
    void macAddress(uint8_t* addr);

    // basic stuff
    void fatalError();

    //multicast
    void setupMultiCast(uint32_t addr, uint16_t port);
    void closeMultiCast();
    bool sendBytes(uint8_t* buffer, uint16_t len);
    int readBytes(uint8_t* buffer, uint16_t maxLen);

    //uart
    virtual void knxUart( HardwareSerial* serial);
    virtual HardwareSerial* knxUart();
    virtual void setupUart();
    virtual void closeUart();
    virtual int uartAvailable();
    virtual size_t writeUart(const uint8_t data);
    virtual size_t writeUart(const uint8_t* buffer, size_t size);
    virtual int readUart();
    virtual size_t readBytesUart(uint8_t* buffer, size_t length);

    static Stream* SerialDebug;

  protected:
    uint32_t _mulitcastAddr;
    uint16_t _mulitcastPort;
    HardwareSerial* _knxSerial;
};
