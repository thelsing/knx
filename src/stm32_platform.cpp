#include "stm32_platform.h"

#ifdef ARDUINO_ARCH_STM32
#include <EEPROM.h>
#include "knx/bits.h"

#ifndef KNX_SERIAL
#define KNX_SERIAL Serial2
#endif

Stm32Platform::Stm32Platform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&KNX_SERIAL)
#endif
{
}

Stm32Platform::Stm32Platform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

Stm32Platform::~Stm32Platform()
{
    delete [] _eepromPtr;
}

uint32_t Stm32Platform::uniqueSerialNumber()
{
    return HAL_GetUIDw0() ^ HAL_GetUIDw1() ^ HAL_GetUIDw2();
}

void Stm32Platform::restart()
{
    NVIC_SystemReset();
}

uint8_t * Stm32Platform::getEepromBuffer(uint32_t size)
{
    // check if the buffer already exists
    if (_eepromPtr == nullptr) // we need to initialize the buffer first
    {
        if (size > E2END + 1)
        {
            fatalError();
        }

        _eepromSize = size;
        _eepromPtr = new uint8_t[size];
        eeprom_buffer_fill();
        for (uint16_t i = 0; i < size; ++i)
            _eepromPtr[i] = eeprom_buffered_read_byte(i);
    }
    
    return _eepromPtr;
}

void Stm32Platform::commitToEeprom()
{
    if(_eepromPtr == nullptr || _eepromSize == 0)
        return;
    for (uint16_t i = 0; i < _eepromSize; ++i)
        eeprom_buffered_write_byte(i, _eepromPtr[i]);
    // For some GD32 chips, the flash needs to be unlocked twice
    // and the first call will fail. If the first call is
    // successful, the second one (inside eeprom_buffer_flush)
    // does nothing.
    HAL_FLASH_Unlock();
    eeprom_buffer_flush();
}

#endif
