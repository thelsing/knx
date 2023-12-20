/*-----------------------------------------------------

Plattform for Raspberry Pi Pico and other RP2040 boards
by SirSydom <com@sirsydom.de> 2021-2022

made to work with arduino-pico - "Raspberry Pi Pico Arduino core, for all RP2040 boards"
by Earl E. Philhower III https://github.com/earlephilhower/arduino-pico 


RTTI must be set to enabled in the board options

Uses direct flash reading/writing.
Size ist defined by KNX_FLASH_SIZE (default 4k) - must be a multiple of 4096.
Offset in Flash is defined by KNX_FLASH_OFFSET (default 1,5MiB / 0x180000) - must be a multiple of 4096.

EEPROM Emulation from arduino-pico core (max 4k) can be use by defining USE_RP2040_EEPROM_EMULATION

A RAM-buffered Flash can be use by defining USE_RP2040_LARGE_EEPROM_EMULATION

For usage of KNX-IP you have to define either
- KNX_IP_W5500 (use the arduino-pico core's w5500 lwip stack)
- KNX_IP_WIFI (use the arduino-pico core's PiPicoW lwip stack)
- KNX_IP_GENERIC (use the Ethernet_Generic stack)

----------------------------------------------------*/

#include "rp2040_arduino_platform.h"

#ifdef ARDUINO_ARCH_RP2040
#include "knx/bits.h"

#include <Arduino.h>

//Pi Pico specific libs
#include <EEPROM.h>             // EEPROM emulation in flash, part of Earl E Philhowers Pi Pico Arduino support 
#include <pico/unique_id.h>     // from Pico SDK
#include <hardware/watchdog.h>  // from Pico SDK
#include <hardware/flash.h>     // from Pico SDK

#define FLASHPTR ((uint8_t*)XIP_BASE + KNX_FLASH_OFFSET)

#ifndef USE_RP2040_EEPROM_EMULATION
#if KNX_FLASH_SIZE%4096
#error "KNX_FLASH_SIZE must be multiple of 4096"
#endif

#if KNX_FLASH_OFFSET%4096
#error "KNX_FLASH_OFFSET must be multiple of 4096"
#endif
#endif

#ifdef KNX_IP_W5500
extern Wiznet5500lwIP KNX_NETIF;
#elif defined(KNX_IP_WIFI)
#elif defined(KNX_IP_GENERIC)

#endif

RP2040ArduinoPlatform::RP2040ArduinoPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&KNX_SERIAL)
#endif
{
    #ifndef USE_RP2040_EEPROM_EMULATION
    _memoryType = Flash;
    #endif
}

RP2040ArduinoPlatform::RP2040ArduinoPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
    #ifndef USE_RP2040_EEPROM_EMULATION
    _memoryType = Flash;
    #endif
}

void RP2040ArduinoPlatform::setupUart()
{
    SerialUART* serial = dynamic_cast<SerialUART*>(_knxSerial);
    if(serial)
    {
        serial->setPollingMode();
    }

    _knxSerial->begin(19200, SERIAL_8E1);
    while (!_knxSerial) 
        ;
}

uint32_t RP2040ArduinoPlatform::uniqueSerialNumber()
{
    pico_unique_board_id_t id;      // 64Bit unique serial number from the QSPI flash

    noInterrupts();
    rp2040.idleOtherCore();

    flash_get_unique_id(id.id);         //pico_get_unique_board_id(&id);

    rp2040.resumeOtherCore();
    interrupts();

    // use lower 4 byte and convert to unit32_t
    uint32_t uid = ((uint32_t)(id.id[4]) << 24) | ((uint32_t)(id.id[5]) << 16) | ((uint32_t)(id.id[6]) << 8) | (uint32_t)(id.id[7]);

    return uid;
}

void RP2040ArduinoPlatform::restart()
{
    println("restart");
    watchdog_reboot(0,0,0);
}

#ifdef USE_RP2040_EEPROM_EMULATION

#pragma warning "Using EEPROM Simulation"

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION

uint8_t * RP2040ArduinoPlatform::getEepromBuffer(uint32_t size)
{
    if(size%4096)
    {
        println("KNX_FLASH_SIZE must be a multiple of 4096");
        fatalError();
    }
    
    if(!_rambuff_initialized)
    {
        memcpy(_rambuff, FLASHPTR, KNX_FLASH_SIZE);
        _rambuff_initialized = true;
    }
    
    return _rambuff;
}

void RP2040ArduinoPlatform::commitToEeprom()
{
    noInterrupts();
    rp2040.idleOtherCore();

    //ToDo: write block-by-block to prevent writing of untouched blocks
    if(memcmp(_rambuff, FLASHPTR, KNX_FLASH_SIZE))
    {
        flash_range_erase (KNX_FLASH_OFFSET, KNX_FLASH_SIZE);
        flash_range_program(KNX_FLASH_OFFSET, _rambuff, KNX_FLASH_SIZE);
    }

    rp2040.resumeOtherCore();
    interrupts();
}

#else

uint8_t * RP2040ArduinoPlatform::getEepromBuffer(uint32_t size)
{
    if(size > 4096)
    {
        println("KNX_FLASH_SIZE to big for EEPROM emulation (max. 4kB)");
        fatalError();
    }
    
    uint8_t * eepromptr = EEPROM.getDataPtr();

    if(eepromptr == nullptr)
    {
        EEPROM.begin(4096);
        eepromptr = EEPROM.getDataPtr();
    }
    
    return eepromptr;
}

void RP2040ArduinoPlatform::commitToEeprom()
{
    EEPROM.commit();
}

#endif

#else

size_t RP2040ArduinoPlatform::flashEraseBlockSize()
{
    return 16; // 16 pages x 256byte/page = 4096byte
}

size_t RP2040ArduinoPlatform::flashPageSize()
{
    return 256;
}

uint8_t* RP2040ArduinoPlatform::userFlashStart()
{
    return (uint8_t*)XIP_BASE + KNX_FLASH_OFFSET;
}

size_t RP2040ArduinoPlatform::userFlashSizeEraseBlocks()
{
    if(KNX_FLASH_SIZE <= 0)
        return 0;
    else
        return ( (KNX_FLASH_SIZE - 1) / (flashPageSize() * flashEraseBlockSize())) + 1;
}

void RP2040ArduinoPlatform::flashErase(uint16_t eraseBlockNum)
{
    noInterrupts();
    rp2040.idleOtherCore();

    flash_range_erase (KNX_FLASH_OFFSET + eraseBlockNum * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());

    rp2040.resumeOtherCore();
    interrupts();
}

void RP2040ArduinoPlatform::flashWritePage(uint16_t pageNumber, uint8_t* data)
{
    noInterrupts();
    rp2040.idleOtherCore();

    flash_range_program(KNX_FLASH_OFFSET + pageNumber * flashPageSize(), data, flashPageSize());

    rp2040.resumeOtherCore();
    interrupts();
}

void RP2040ArduinoPlatform::writeBufferedEraseBlock()
{
    if(_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
    {
        noInterrupts();
        rp2040.idleOtherCore();

        flash_range_erase (KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());
        flash_range_program(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), _eraseblockBuffer, flashPageSize() * flashEraseBlockSize());

        rp2040.resumeOtherCore();
        interrupts();

        _bufferedEraseblockDirty = false;
    }
}
#endif

#if defined(KNX_NETIF)
uint32_t RP2040ArduinoPlatform::currentIpAddress()
{

    return KNX_NETIF.localIP();
}
uint32_t RP2040ArduinoPlatform::currentSubnetMask()
{
    return KNX_NETIF.subnetMask();
}
uint32_t RP2040ArduinoPlatform::currentDefaultGateway()
{
    return KNX_NETIF.gatewayIP();
}
void RP2040ArduinoPlatform::macAddress(uint8_t* addr)
{
#if defined(KNX_IP_W5500)
    addr = KNX_NETIF.getNetIf()->hwaddr;
#elif defined(KNX_IP_WIFI)
    uint8_t macaddr[6] = {0,0,0,0,0,0};
    addr = KNX_NETIF.macAddress(macaddr);
#elif defined(KNX_IP_GENERIC)
    KNX_NETIF.MACAddress(addr);
#endif
}

// multicast
void RP2040ArduinoPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    mcastaddr = IPAddress(htonl(addr));
    _port = port;
    uint8_t result = _udp.beginMulticast(mcastaddr, port);
    (void) result;

    #ifdef KNX_IP_GENERIC
    //if(!_unicast_socket_setup)
    //    _unicast_socket_setup = UDP_UNICAST.begin(3671);
    #endif

#ifdef KNX_LOG_IP
    print("Setup Mcast addr: ");
    print(mcastaddr.toString().c_str());
    print(" on port: ");
    print(port);
    print(" result ");
    println(result);
#endif
}

void RP2040ArduinoPlatform::closeMultiCast()
{
    _udp.stop();
}

bool RP2040ArduinoPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
{
#ifdef KNX_LOG_IP
    printHex("<- ",buffer, len);
#endif
    //ToDo: check if Ethernet is able to receive
    _udp.beginPacket(mcastaddr, _port);
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}

int RP2040ArduinoPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();
    if (len == 0)
        return 0;

    if (len > maxLen)
    {
        print("udp buffer to small. was ");
        print(maxLen);
        print(", needed ");
        println(len);
        fatalError();
    }

    _udp.read(buffer, len);
#ifdef KNX_LOG_IP
    print("Remote IP: ");
    print(_udp.remoteIP().toString().c_str());

    printHex("-> ", buffer, len);
#endif
    return len;
}

// unicast
bool RP2040ArduinoPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    IPAddress ucastaddr(htonl(addr));
    
#ifdef KNX_LOG_IP
    print("sendBytesUniCast to:");
    println(ucastaddr.toString().c_str());
#endif

#ifdef KNX_IP_GENERIC
    if(!_unicast_socket_setup)
        _unicast_socket_setup = UDP_UNICAST.begin(3671);
#endif

    if (UDP_UNICAST.beginPacket(ucastaddr, port) == 1)
    {
        UDP_UNICAST.write(buffer, len);
        if (UDP_UNICAST.endPacket() == 0)
            println("sendBytesUniCast endPacket fail");
    }
    else
        println("sendBytesUniCast beginPacket fail");

    return true;
}
#endif

#endif


