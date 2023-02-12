#ifdef ARDUINO_ARCH_ESP8266
#include "arduino_platform.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


class EspPlatform : public ArduinoPlatform
{
  public:
    EspPlatform();
    EspPlatform(HardwareSerial* s);

    // ip stuff
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // unique serial number
    uint32_t uniqueSerialNumber() override;

    // basic stuff
    void restart();

    //multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
    int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen) override;
   
    //unicast 
    bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;
    
    //memory
    uint8_t* getEepromBuffer(uint32_t size);
    void commitToEeprom();
private:
    WiFiUDP _udp;
    uint32_t _multicastAddr;
    uint16_t _multicastPort;
};

#endif
