#include "esp32_platform.h"

#ifdef ARDUINO_ARCH_ESP32
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

Esp32Platform::Esp32Platform() : ArduinoPlatform(&Serial1)
{
}

Esp32Platform::Esp32Platform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

uint32_t Esp32Platform::currentIpAddress()
{
    return WiFi.localIP();
}

uint32_t Esp32Platform::currentSubnetMask()
{
    return WiFi.subnetMask();
}

uint32_t Esp32Platform::currentDefaultGateway()
{
    return WiFi.gatewayIP();
}

void Esp32Platform::macAddress(uint8_t * addr)
{
    esp_wifi_get_mac(WIFI_IF_STA, addr);
}

void Esp32Platform::restart()
{
    println("restart");
    ESP.restart();
}

void Esp32Platform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _mulitcastAddr = htonl(addr);
    _mulitcastPort = port;
    IPAddress mcastaddr(_mulitcastAddr);
    
    Serial.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8_t result = _udp.beginMulticast(mcastaddr, port);
    Serial.printf("result %d\n", result);
}

void Esp32Platform::closeMultiCast()
{
    _udp.stop();
}

bool Esp32Platform::sendBytes(uint8_t * buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    int result = 0;
    result = _udp.beginMulticastPacket();
    result = _udp.write(buffer, len);
    result = _udp.endPacket();
    return true;
}

int Esp32Platform::readBytes(uint8_t * buffer, uint16_t maxLen)
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

bool Esp32Platform::writeNVMemory(uintptr_t addr,uint8_t data)
{
    *((uint8_t*)addr) = data;
    return true;
}

uint8_t Esp32Platform::readNVMemory(uintptr_t addr)
{
    return *((uint8_t*)addr);
}

uintptr_t Esp32Platform::allocNVMemory(size_t size,uint32_t ID)
{
    return (uintptr_t)EEPROM.getDataPtr();
}

uintptr_t Esp32Platform::reloadNVMemory(uint32_t ID)
{
    EEPROM.begin(1024);
    return (uintptr_t)EEPROM.getDataPtr();
}

void Esp32Platform::finishNVMemory()
{
    EEPROM.commit();
}

void Esp32Platform::freeNVMemory(uint32_t ID)
{
}
#endif
