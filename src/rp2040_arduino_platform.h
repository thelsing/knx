#pragma once

#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_RP2040

#ifndef USE_RP2040_EEPROM_EMULATION
#ifndef KNX_FLASH_OFFSET
#define KNX_FLASH_OFFSET 0x180000   // 1.5MiB
#pragma warning "KNX_FLASH_OFFSET not defined, using 0x180000"
#endif
#endif

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION
#define USE_RP2040_EEPROM_EMULATION
#endif

#ifndef KNX_SERIAL
#pragma warn "KNX_SERIAL not defined, using Serial1"
#define KNX_SERIAL Serial1
#endif

#ifdef KNX_IP_W5500
#if ARDUINO_PICO_MAJOR * 10000 + ARDUINO_PICO_MINOR * 100 + ARDUINO_PICO_REVISION < 30600
#pragma error "arduino-pico >= 3.6.0 needed"
#endif
#define KNX_NETIF Eth

#include "SPI.h"
#include <W5500lwIP.h>

#elif defined(KNX_IP_WIFI)

#define KNX_NETIF WiFi
#include <WiFi.h>

#elif defined(KNX_IP_GENERIC)


#include <SPI.h>

#ifndef DEBUG_ETHERNET_GENERIC_PORT
#define DEBUG_ETHERNET_GENERIC_PORT         Serial
#endif

#ifndef _ETG_LOGLEVEL_
#define _ETG_LOGLEVEL_                      1
#endif


#define ETHERNET_USE_RPIPICO      true
#include <Ethernet_Generic.hpp>             // https://github.com/khoih-prog/Ethernet_Generic


#define KNX_NETIF Ethernet

#endif


class RP2040ArduinoPlatform : public ArduinoPlatform
{
public:
    RP2040ArduinoPlatform();
    RP2040ArduinoPlatform( HardwareSerial* s);

    // uart
    void knxUartPins(pin_size_t rxPin, pin_size_t txPin);
    void setupUart();

    // unique serial number
    uint32_t uniqueSerialNumber() override; 

    void restart();

    #ifdef USE_RP2040_EEPROM_EMULATION
    uint8_t* getEepromBuffer(uint32_t size);
    void commitToEeprom();

    #ifdef USE_RP2040_LARGE_EEPROM_EMULATION
    uint8_t _rambuff[KNX_FLASH_SIZE];
    bool _rambuff_initialized = false;
    #endif
    #else

    // size of one EraseBlock in pages
    virtual size_t flashEraseBlockSize();
    // size of one flash page in bytes
    virtual size_t flashPageSize();
    // start of user flash aligned to start of an erase block
    virtual uint8_t* userFlashStart();
    // size of the user flash in EraseBlocks
    virtual size_t userFlashSizeEraseBlocks();
    //relativ to userFlashStart
    virtual void flashErase(uint16_t eraseBlockNum);
    //write a single page to flash (pageNumber relative to userFashStart
    virtual void flashWritePage(uint16_t pageNumber, uint8_t* data); 
    
    // writes _eraseblockBuffer to flash - overrides Plattform::writeBufferedEraseBlock() for performance optimization only
    void writeBufferedEraseBlock();
    #endif


    #if defined(KNX_NETIF)
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
    int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen) override;

    // unicast
    bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;

    #if defined(KNX_IP_W5500) || defined(KNX_IP_WIFI)
    #define UDP_UNICAST _udp
    protected: WiFiUDP _udp;
    #elif defined(KNX_IP_GENERIC)
    #define UDP_UNICAST _udp_uni
    protected: bool _unicast_socket_setup = false;
    protected: EthernetUDP _udp;
    protected: EthernetUDP UDP_UNICAST;
    #endif
    protected: IPAddress mcastaddr;
    protected: uint16_t _port;
    #endif
    protected: pin_size_t _rxPin = UART_PIN_NOT_DEFINED; 
    protected: pin_size_t _txPin = UART_PIN_NOT_DEFINED;
};

#endif
