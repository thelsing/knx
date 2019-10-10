#ifdef ARDUINO_ARCH_ESP8266
#include "arduino_platform.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


class EspPlatform : public ArduinoPlatform
{
    using ArduinoPlatform::_mulitcastAddr;
    using ArduinoPlatform::_mulitcastPort;

  public:
    EspPlatform();
    EspPlatform( HardwareSerial* s);

    // ip stuff
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // basic stuff
    void restart();

    //multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytes(uint8_t* buffer, uint16_t len) override;
    int readBytes(uint8_t* buffer, uint16_t maxLen) override;
   
    //memory
    bool writeNVMemory(uintptr_t addr,uint8_t data);
    uint8_t readNVMemory(uintptr_t addr);
    uintptr_t allocNVMemory(size_t size,uint32_t ID);
    uintptr_t reloadNVMemory(uint32_t ID);
    void finishNVMemory();
    void freeNVMemory(uint32_t ID);
private:
    WiFiUDP _udp;
};

#endif
