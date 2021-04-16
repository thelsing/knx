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

uint32_t SamdPlatform::uniqueSerialNumber()
{
    #if defined (__SAMD51__)
      // SAMD51 from section 9.6 of the datasheet
      #define SERIAL_NUMBER_WORD_0	*(volatile uint32_t*)(0x008061FC)
      #define SERIAL_NUMBER_WORD_1	*(volatile uint32_t*)(0x00806010)
      #define SERIAL_NUMBER_WORD_2	*(volatile uint32_t*)(0x00806014)
      #define SERIAL_NUMBER_WORD_3	*(volatile uint32_t*)(0x00806018)
    #else
    //#elif defined (__SAMD21E17A__) || defined(__SAMD21G18A__)  || defined(__SAMD21E18A__) || defined(__SAMD21J18A__)
    // SAMD21 from section 9.3.3 of the datasheet
      #define SERIAL_NUMBER_WORD_0	*(volatile uint32_t*)(0x0080A00C)
      #define SERIAL_NUMBER_WORD_1	*(volatile uint32_t*)(0x0080A040)
      #define SERIAL_NUMBER_WORD_2	*(volatile uint32_t*)(0x0080A044)
      #define SERIAL_NUMBER_WORD_3	*(volatile uint32_t*)(0x0080A048)
    #endif

    return SERIAL_NUMBER_WORD_0 ^ SERIAL_NUMBER_WORD_1 ^ SERIAL_NUMBER_WORD_2 ^ SERIAL_NUMBER_WORD_3;
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


