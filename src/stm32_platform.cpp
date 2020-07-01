#include "stm32_platform.h"

#ifdef ARDUINO_ARCH_STM32
#include <stm32_eeprom.h>
#include "knx/bits.h"

Stm32Platform::Stm32Platform() : ArduinoPlatform(&Serial2)
{
}

Stm32Platform::Stm32Platform( HardwareSerial* s) : ArduinoPlatform(s)
{
}

Stm32Platform::~Stm32Platform()
{
    delete [] _eepromPtr;
}

void Stm32Platform::restart()
{
    NVIC_SystemReset();
}

uint8_t * Stm32Platform::getEepromBuffer(uint16_t size)
{
    if (size > E2END + 1)
    {
        fatalError();
    }
    _eepromSize = size;
    delete [] _eepromPtr;
    _eepromPtr = new uint8_t[size];
    eeprom_buffer_fill();
    for (uint16_t i = 0; i < size; ++i)
        _eepromPtr[i] = eeprom_buffered_read_byte(i);
    return _eepromPtr;
}

void Stm32Platform::commitToEeprom()
{
    if(_eepromPtr == nullptr || _eepromSize == 0)
        return;
    for (uint16_t i = 0; i < _eepromSize; ++i)
        eeprom_buffered_write_byte(i, _eepromPtr[i]);
    eeprom_buffer_flush();
}

#endif
