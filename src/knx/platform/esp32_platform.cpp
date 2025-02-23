#ifdef ARDUINO_ARCH_ESP32
#pragma once
#include "esp32_platform.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "knx/bits.h"

#ifndef KNX_SERIAL
    #define KNX_SERIAL Serial1
    #pragma warn "KNX_SERIAL not defined, using Serial1"
#endif
 
#ifdef KNX_IP_LAN
    #include "ETH.h"
    #define KNX_NETIF ETH
#else // KNX_IP_WIFI
    #include <WiFi.h>
    #define KNX_NETIF WiFi
#endif


namespace Knx
{
    Esp32Platform::Esp32Platform()
#ifndef KNX_NO_DEFAULT_UART
        : ArduinoPlatform(&KNX_SERIAL)
#endif
    {
#ifdef KNX_UART_RX_PIN
        _rxPin = KNX_UART_RX_PIN;
#endif
#ifdef KNX_UART_TX_PIN
        _txPin = KNX_UART_TX_PIN;
#endif
    }

    Esp32Platform::Esp32Platform(HardwareSerial* s) : ArduinoPlatform(s)
    {
    }

    void Esp32Platform::knxUartPins(int8_t rxPin, int8_t txPin)
    {
        _rxPin = rxPin;
        _txPin = txPin;
    }

    // ESP specific uart handling with pins
    void Esp32Platform::setupUart()
    {
        _knxSerial->begin(19200, SERIAL_8E1, _rxPin, _txPin);

        while (!_knxSerial)
            ;
    }

    uint32_t Esp32Platform::currentIpAddress()
    {
        return KNX_NETIF.localIP();
    }

    uint32_t Esp32Platform::currentSubnetMask()
    {
        return KNX_NETIF.subnetMask();
    }

    uint32_t Esp32Platform::currentDefaultGateway()
    {
        return KNX_NETIF.gatewayIP();
    }

    void Esp32Platform::macAddress(uint8_t* addr)
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

        KNX_DEBUG_SERIAL.printf("setup multicast addr: %s port: %d ip: %s\n", mcastaddr.toString().c_str(), port,
                                KNX_NETIF.localIP().toString().c_str());
        uint8_t result = _udp.beginMulticast(mcastaddr, port);
        KNX_DEBUG_SERIAL.printf("result %d\n", result);
    }

    void Esp32Platform::closeMultiCast()
    {
        _udp.stop();
    }

    bool Esp32Platform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
    {
        //printHex("<- ",buffer, len);
        _udp.beginMulticastPacket();
        _udp.write(buffer, len);
        _udp.endPacket();
        return true;
    }

    int Esp32Platform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port)
    {
        int len = _udp.parsePacket();

        if (len == 0)
            return 0;

        if (len > maxLen)
        {
            println("Unexpected UDP data packet length - drop packet");

            for (size_t i = 0; i < len; i++)
                _udp.read();

            return 0;
        }

        _udp.read(buffer, len);
        _remoteIP = _udp.remoteIP();
        _remotePort = _udp.remotePort();
        src_addr = htonl(_remoteIP);
        src_port = _remotePort;

        // print("Remote IP: ");
        // print(_udp.remoteIP().toString().c_str());
        // printHex("-> ", buffer, len);

        return len;
    }

    bool Esp32Platform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
    {
        IPAddress ucastaddr(htonl(addr));

        if (!addr)
            ucastaddr = _remoteIP;

        if (!port)
            port = _remotePort;

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

    uint8_t* Esp32Platform::getEepromBuffer(uint32_t size)
    {
        uint8_t* eepromptr = EEPROM.getDataPtr();

        if (eepromptr == nullptr)
        {
            EEPROM.begin(size);
            eepromptr = EEPROM.getDataPtr();
        }

        return eepromptr;
    }

    void Esp32Platform::commitToEeprom()
    {
        EEPROM.getDataPtr(); // trigger dirty flag in EEPROM lib to make sure data will be written to flash
        EEPROM.commit();
    }
}
#endif