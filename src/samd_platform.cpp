#include "samd_platform.h"

#ifdef ARDUINO_ARCH_SAMD
#include <knx/bits.h>

#include <Arduino.h>
#include <FlashAsEEPROM.h>

SamdPlatform::SamdPlatform() : ArduinoPlatform(&Serial1)
{
}

SamdPlatform::SamdPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

void SamdPlatform::restart()
{
    ArduinoPlatform::SerialDebug.println("restart");
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


