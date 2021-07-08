/*-----------------------------------------------------

Plattform for Raspberry Pi Pico and other RP2040 boards

made to work with arduino-pico - "Raspberry Pi Pico Arduino core, for all RP2040 boards"
by Earl E. Philhower III https://github.com/earlephilhower/arduino-pico
tested with V1.9.1

by SirSydom <com@sirsydom.de> 2021

A maximum of 4kB emulated EEPROM is supported.
For more, use or own emulation (maybe with littlefs)

----------------------------------------------------*/

#include "rp2040_arduino_platform.h"

#ifdef ARDUINO_ARCH_RP2040
#include <knx/bits.h>

#include <Arduino.h>

//Pi Pico specific libs
#include <EEPROM.h>             // EEPROM emulation in flash, part of Earl E Philhowers Pi Pico Arduino support 
#include <pico/unique_id.h>     // from Pico SDK
#include <hardware/watchdog.h>  // from Pico SDK


RP2040ArduinoPlatform::RP2040ArduinoPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&Serial1)
#endif
{
}

RP2040ArduinoPlatform::RP2040ArduinoPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

uint32_t RP2040ArduinoPlatform::uniqueSerialNumber()
{
    pico_unique_board_id_t id;      // 64Bit unique serial number from the QSPI flash
    pico_get_unique_board_id(&id);

    // use lower 4 byte and convert to unit32_t
    uint32_t uid = ((uint32_t)(id.id[4]) << 24) | ((uint32_t)(id.id[5]) << 16) | ((uint32_t)(id.id[6]) << 8) | (uint32_t)(id.id[7]);

    return uid;
}

void RP2040ArduinoPlatform::restart()
{
    println("restart");
    watchdog_reboot(0,0,0);
}

uint8_t * RP2040ArduinoPlatform::getEepromBuffer(uint16_t size)
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


