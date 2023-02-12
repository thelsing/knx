#include "esp_platform.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <user_interface.h>
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

#ifndef KNX_SERIAL
#define KNX_SERIAL Serial
#endif

EspPlatform::EspPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&KNX_SERIAL)
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
    
    KNX_DEBUG_SERIAL.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
        WiFi.localIP().toString().c_str());
    uint8 result = _udp.beginMulticast(WiFi.localIP(), mcastaddr, port);
    KNX_DEBUG_SERIAL.printf("result %d\n", result);
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
        KNX_DEBUG_SERIAL.printf("udp buffer to small. was %d, needed %d\n", maxLen, len);
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

uint8_t * EspPlatform::getEepromBuffer(uint32_t size)
{
    uint8_t * eepromptr = EEPROM.getDataPtr();
    if(eepromptr == nullptr) {
        EEPROM.begin(size);
        eepromptr = EEPROM.getDataPtr();
    }
    return eepromptr;
}

void EspPlatform::commitToEeprom()
{
    EEPROM.commit();
}
#endif
