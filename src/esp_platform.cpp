#include "esp_platform.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <user_interface.h>
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

EspPlatform::EspPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&Serial)
#endif
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

uint32_t EspPlatform::uniqueSerialNumber()
{
    return ESP.getChipId();
}

void EspPlatform::restart()
{
    println("restart");
    ESP.reset();
}

void EspPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _multicastAddr = htonl(addr);
    _multicastPort = port;
    IPAddress mcastaddr(_multicastAddr);
    
    Serial.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8 result = _udp.beginMulticast(WiFi.localIP(), mcastaddr, port);
    Serial.printf("result %d\n", result);
}

void EspPlatform::closeMultiCast()
{
    _udp.stop();
}

bool EspPlatform::sendBytesMultiCast(uint8_t * buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    _udp.beginPacketMulticast(_multicastAddr, _multicastPort, WiFi.localIP());
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}

int EspPlatform::readBytesMultiCast(uint8_t * buffer, uint16_t maxLen)
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

bool EspPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
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

uint8_t * EspPlatform::getEepromBuffer(uint16_t size)
{
    EEPROM.begin(size);
    return EEPROM.getDataPtr();
}

void EspPlatform::commitToEeprom()
{
    EEPROM.commit();
}
#endif
