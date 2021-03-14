#include "samd_platform.h"

#ifdef ARDUINO_ARCH_SAMD
#include <knx/bits.h>

#include <Arduino.h>
#include <FlashAsEEPROM.h>

SamdPlatform::SamdPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&Serial1)
#endif
{
}

SamdPlatform::SamdPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

void SamdPlatform::restart()
{
    println("restart");
    NVIC_SystemReset();
}

uint8_t * SamdPlatform::getEepromBuffer(uint16_t size)
{
    //EEPROM.begin(size);
    if(size > EEPROM_EMULATION_SIZE)
        fatalError();
    
    return EEPROM.getDataPtr();
}

void SamdPlatform::commitToEeprom()
{
    EEPROM.commit();
}
#endif


