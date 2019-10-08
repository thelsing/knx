#include "samd_platform.h"

#ifdef ARDUINO_ARCH_SAMD
#include <knx/bits.h>

#include <Arduino.h>
#include "samd_flash.h"

SamdFlash Flash;

SamdPlatform::SamdPlatform() : ArduinoPlatform(&Serial1)
{
    Platform::_NVMemoryType = internalFlash;
}

SamdPlatform::SamdPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
    Platform::_NVMemoryType = internalFlash;
}

void SamdPlatform::restart()
{
    println("restart");
    NVIC_SystemReset();
}


bool SamdPlatform::writeNVMemory(uint32_t addr,uint8_t data)
{
    if(Flash.write((uint8_t*)addr, data)==false)
        fatalError();
    return true;
}

uint8_t SamdPlatform::readNVMemory(uint32_t addr)
{
    return Flash.read((uint8_t*)addr);
}

uint32_t SamdPlatform::allocNVMemory(uint32_t size,uint32_t ID)
{
    uint32_t addr = (uint32_t)Flash.malloc(size, ID);
    if(addr == 0)
        fatalError();
    return addr;
}

uint32_t SamdPlatform::reloadNVMemory(uint32_t ID)
{
 //  Flash.erase();
   return (uint32_t)Flash.loadBlock(ID);
}

void SamdPlatform::finishNVMemory()
{
    Flash.finalise();
}

void SamdPlatform::freeNVMemory(uint32_t ID)
{
    Flash.free(ID);
  //  Flash.erase();
}

uint8_t* SamdPlatform::memoryReference()
{
    return (uint8_t*)Flash.getStartAddress();
}



/*************_NVMemoryType = internalRam*************************

bool SamdPlatform::writeNVMemory(uint32_t addr,uint8_t data)
{
    *((uint8_t*)addr) = data;
    return true;
}

uint8_t SamdPlatform::readNVMemory(uint32_t addr)
{
    return *((uint8_t*)addr);
}

uint32_t SamdPlatform::allocNVMemory(uint32_t size,uint32_t ID)
{
    if(size > EEPROM_EMULATION_SIZE)
        fatalError();
    return (uint32_t)EEPROM.getDataPtr();
}

uint32_t SamdPlatform::reloadNVMemory(uint32_t ID)
{
    return (uint32_t)EEPROM.getDataPtr();
}

void SamdPlatform::finishNVMemory()
{
    EEPROM.commit();
}

void SamdPlatform::freeNVMemory(uint32_t ID)
{
}
/*

/*************_NVMemoryType = external*************************
bool SamdPlatform::writeNVMemory(uint32_t addr,uint8_t data)
{
    EEPROM.write(addr-1, data);
    return true;
}

uint8_t SamdPlatform::readNVMemory(uint32_t addr)
{
    return EEPROM.read(addr-1);
}

uint32_t SamdPlatform::allocNVMemory(uint32_t size,uint32_t ID)
{
    if(size > EEPROM_EMULATION_SIZE)
        fatalError();
    return 1;
}

uint32_t SamdPlatform::reloadNVMemory(uint32_t ID)
{
    return 1;
}

void SamdPlatform::finishNVMemory()
{
    EEPROM.commit();
}

void SamdPlatform::freeNVMemory(uint32_t ID)
{
}

*/

#endif


