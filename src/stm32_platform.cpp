#include "stm32_platform.h"

#ifdef ARDUINO_ARCH_STM32
#include <EEPROM.h>
#include "knx/bits.h"

Stm32Platform::Stm32Platform() : ArduinoPlatform(&Serial2), eepromPtr(nullptr), eepromSize(0)
{
}

Stm32Platform::Stm32Platform( HardwareSerial* s) : ArduinoPlatform(s), eepromPtr(nullptr), eepromSize(0)
{
}

Stm32Platform::~Stm32Platform()
{
	delete [] eepromPtr;
}

void Stm32Platform::restart()
{
	NVIC_SystemReset();
}

uint8_t * Stm32Platform::getEepromBuffer(uint16_t size)
{
	delete [] eepromPtr;
	eepromSize = size;
	eepromPtr = new uint8_t[size];
	for (uint16_t i = 0; i < size; ++i)
		eepromPtr[i] = EEPROM[i];
	return eepromPtr;
}

void Stm32Platform::commitToEeprom()
{
	if(eepromPtr == nullptr)
		return;
	for (uint16_t i = 0; i < eepromSize; ++i)
		EEPROM.update(i, eepromPtr[i]);
}

#endif
