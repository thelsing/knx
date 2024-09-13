#ifdef ARDUINO_ARCH_ESP8266
#include "esp8266_platform.h"


#include <user_interface.h>
#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

#ifndef KNX_SERIAL
    #define KNX_SERIAL Serial
#endif

namespace Knx
{
    Esp8266Platform::Esp8266Platform()
#ifndef KNX_NO_DEFAULT_UART
        : ArduinoPlatform(&KNX_SERIAL)
#endif
    {
    }

    Esp8266Platform::Esp8266Platform( HardwareSerial* s) : ArduinoPlatform(s)
    {
    }

    uint32_t Esp8266Platform::currentIpAddress()
    {
        return WiFi.localIP();
    }

    uint32_t Esp8266Platform::currentSubnetMask()
    {
        return WiFi.subnetMask();
    }

    uint32_t Esp8266Platform::currentDefaultGateway()
    {
        return WiFi.gatewayIP();
    }

    void Esp8266Platform::macAddress(uint8_t* addr)
    {
        wifi_get_macaddr(STATION_IF, addr);
    }

    uint32_t Esp8266Platform::uniqueSerialNumber()
    {
        return ESP.getChipId();
    }

    void Esp8266Platform::restart()
    {
        println("restart");
        ESP.reset();
    }

    void Esp8266Platform::setupMultiCast(uint32_t addr, uint16_t port)
    {
        _multicastAddr = htonl(addr);
        _multicastPort = port;
        IPAddress mcastaddr(_multicastAddr);

        KNX_DEBUG_SERIAL.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
                                WiFi.localIP().toString().c_str());
        uint8 result = _udp.beginMulticast(WiFi.localIP(), mcastaddr, port);
        KNX_DEBUG_SERIAL.printf("result %d\n", result);
    }

    void Esp8266Platform::closeMultiCast()
    {
        _udp.stop();
    }

    bool Esp8266Platform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
    {
        //printHex("<- ",buffer, len);
        _udp.beginPacketMulticast(_multicastAddr, _multicastPort, WiFi.localIP());
        _udp.write(buffer, len);
        _udp.endPacket();
        return true;
    }

    int Esp8266Platform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen)
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

    bool Esp8266Platform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
    {
        IPAddress ucastaddr(htonl(addr));
        println("sendBytesUniCast endPacket fail");

        if (_udp.beginPacket(ucastaddr, port) == 1)
        {
            _udp.write(buffer, len);

            if (_udp.endPacket() == 0)
                println("sendBytesUniCast endPacket fail");
        }
        else
            println("sendBytesUniCast beginPacket fail");

        return true;
    }

    uint8_t* Esp8266Platform::getEepromBuffer(uint32_t size)
    {
        uint8_t* eepromptr = EEPROM.getDataPtr();

        if (eepromptr == nullptr)
        {
            EEPROM.begin(size);
            eepromptr = EEPROM.getDataPtr();
        }

        return eepromptr;
    }

    void Esp8266Platform::commitToEeprom()
    {
        EEPROM.commit();
    }
}
#endif