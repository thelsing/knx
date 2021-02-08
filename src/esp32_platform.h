#ifdef ARDUINO_ARCH_ESP32
#include "arduino_platform.h"
#include <WiFi.h>
#include <WiFiUdp.h>


class Esp32Platform : public ArduinoPlatform
{
public:
    Esp32Platform();
    Esp32Platform(HardwareSerial* s);

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
    bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
    int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen) override;
    
    //memory
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();
private:
    WiFiUDP _udp;
};

#endif
