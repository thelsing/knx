#include "esp_platform.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <user_interface.h>
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

EspPlatform::EspPlatform() : ArduinoPlatform(&Serial)
{
}

EspPlatform::EspPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

uint32_t EspPlatform::currentIpAddress()
{
    return WiFi.localIP();
}

uint32_t EspPlatform::currentSubnetMask()
{
    return WiFi.subnetMask();
}

uint32_t EspPlatform::currentDefaultGateway()
{
    return WiFi.gatewayIP();
}

void EspPlatform::macAddress(uint8_t * addr)
{
    wifi_get_macaddr(STATION_IF, addr);
}

void EspPlatform::restart()
{
    println("restart");
    ESP.reset();
}

void EspPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _mulitcastAddr = htonl(addr);
    _mulitcastPort = port;
    IPAddress mcastaddr(_mulitcastAddr);
    
    Serial.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8 result = _udp.beginMulticast(WiFi.localIP(), mcastaddr, port);
    Serial.printf("result %d\n", result);
}

void EspPlatform::closeMultiCast()
{
    _udp.stop();
}

bool EspPlatform::sendBytes(uint8_t * buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    int result = 0;
    result = _udp.beginPacketMulticast(_mulitcastAddr, _mulitcastPort, WiFi.localIP());
    result = _udp.write(buffer, len);
    result = _udp.endPacket();
    return true;
}

int EspPlatform::readBytes(uint8_t * buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();
    if (len == 0)
        return 0;
    
    if (len > maxLen)
    {
        Serial.printf("udp buffer to small. was %d, needed %d\n", maxLen, len);
        fatalError();
    }

    _udp.read(buffer, len);
    //printHex("-> ", buffer, len);
    return len;
}

bool EspPlatform::writeNVMemory(uintptr_t addr,uint8_t data)
{
    *((uint8_t*)addr) = data;
    return true;
}

uint8_t EspPlatform::readNVMemory(uintptr_t addr)
{
    return *((uint8_t*)addr);
}

uintptr_t EspPlatform::allocNVMemory(size_t size,uint32_t ID)
{
    return (uintptr_t)EEPROM.getDataPtr();
}

uintptr_t EspPlatform::reloadNVMemory(uint32_t ID)
{
    EEPROM.begin(1024);
    return (uintptr_t)EEPROM.getDataPtr();
}

void EspPlatform::finishNVMemory()
{
    EEPROM.commit();
}

void EspPlatform::freeNVMemory(uint32_t ID)
{
}
#endif
