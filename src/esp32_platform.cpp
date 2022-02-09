#include "esp32_platform.h"

#ifdef ARDUINO_ARCH_ESP32
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

Esp32Platform::Esp32Platform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&Serial1)
#endif
{
}

Esp32Platform::Esp32Platform(HardwareSerial* s) : ArduinoPlatform(s)
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

uint32_t Esp32Platform::uniqueSerialNumber()
{
    uint64_t chipid = ESP.getEfuseMac();
    uint32_t upperId = (chipid >> 32) & 0xFFFFFFFF;
    uint32_t lowerId = (chipid & 0xFFFFFFFF);
    return (upperId ^ lowerId);
}

void Esp32Platform::restart()
{
    println("restart");
    ESP.restart();
}

void Esp32Platform::setupMultiCast(uint32_t addr, uint16_t port)
{
    IPAddress mcastaddr(htonl(addr));
    
    Serial.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8_t result = _udp.beginMulticast(mcastaddr, port);
    Serial.printf("result %d\n", result);
}

void Esp32Platform::closeMultiCast()
{
    _udp.stop();
}

bool Esp32Platform::sendBytesMultiCast(uint8_t * buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    _udp.beginMulticastPacket();
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}

int Esp32Platform::readBytesMultiCast(uint8_t * buffer, uint16_t maxLen)
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

bool Esp32Platform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    IPAddress ucastaddr(htonl(addr));
    println("sendBytesUniCast endPacket fail");
    if(_udp.beginPacket(ucastaddr, port) == 1) {
        _udp.write(buffer, len);
        if(_udp.endPacket() == 0) println("sendBytesUniCast endPacket fail");
    }
    else println("sendBytesUniCast beginPacket fail");
    return true;
}

uint8_t * Esp32Platform::getEepromBuffer(uint16_t size)
{
    EEPROM.begin(size);
    return EEPROM.getDataPtr();
}

void Esp32Platform::commitToEeprom()
{
    EEPROM.getDataPtr(); // trigger dirty flag in EEPROM lib to make sure data will be written to flash
    EEPROM.commit();
}

#endif
