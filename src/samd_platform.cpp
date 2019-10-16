#include "samd_platform.h"

#ifdef ARDUINO_ARCH_SAMD
#include <knx/bits.h>

#include <Arduino.h>
#ifdef INTERN_FLASH_MEMORY
#include "samd_flash.h"
SamdFlash Flash;
#endif
#ifdef EXTERN_EEPROM_MEMORY
#include "FlashAsEEPROM.h"
#endif
#ifdef RAM_EMULATED_MEMORY
#include "FlashAsEEPROM.h"
#endif



SamdPlatform::SamdPlatform() : ArduinoPlatform(&Serial1)
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

#ifdef INTERN_FLASH_MEMORY
bool SamdPlatform::writeNVMemory(uint8_t* addr,uint8_t data)
{
    if(Flash.write(addr, data)==false)
        fatalError();
    return true;
}

uint8_t SamdPlatform::readNVMemory(uint8_t* addr)
{
    return Flash.read(addr);
}

uint8_t* SamdPlatform::allocNVMemory(size_t size,uint32_t ID)
{
    uint8_t* addr = Flash.malloc(size, ID);
    if(addr == 0)
        fatalError();
    return addr;
}

uint8_t* SamdPlatform::reloadNVMemory(uint32_t ID, bool pointerAccess)
{
 //  Flash.erase();
   return Flash.loadBlock(ID);
}

void SamdPlatform::finishNVMemory()
{
    Flash.finalise();
}

void SamdPlatform::freeNVMemory(uint32_t ID)
{
    Flash.free(ID);
}

uint8_t* SamdPlatform::referenceNVMemory()
{
    return (uint8_t*)Flash.getStartAddress();
}
#endif

#ifdef EXTERN_EEPROM_MEMORY
bool SamdPlatform::writeNVMemory(uint8_t* addr,uint8_t data)
{
    *addr = data;
    return true;
}

uint8_t SamdPlatform::readNVMemory(uint8_t* addr)
{
    return *addr;
}

uint8_t* SamdPlatform::allocNVMemory(size_t size,uint32_t ID)
{
    int i;
    for(i=0;i<MAX_MEMORY_BLOCKS;i++){
        if(_memoryBlocks[i].ID == 0)
            break;
    }
    if(i >= MAX_MEMORY_BLOCKS)
        fatalError();


    _memoryBlocks[i].data = (uint8_t*)malloc(size);
    if(_memoryBlocks[i].data == NULL)
        fatalError();

    _memoryBlocks[i].ID = ID;
    _memoryBlocks[i].size = size;

    return _memoryBlocks[i].data;
}

void SamdPlatform::initNVMemory()
{
    uint32_t addr = 0;
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++){

        if (EEPROM.read(addr++) != 0xBA || EEPROM.read(addr++) != 0xAD || EEPROM.read(addr++) != 0xC0 || EEPROM.read(addr++) != 0xDE){
            _memoryBlocks[i].ID = 0;
            _memoryBlocks[i].size = 0;
            _memoryBlocks[i].data = NULL;
            continue;
        }

        ((uint8_t*)&_memoryBlocks[i].ID)[0] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[1] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[2] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[3] = EEPROM.read(addr++);

        ((uint8_t*)&_memoryBlocks[i].size)[0] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[1] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[2] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[3] = EEPROM.read(addr++);

        _memoryBlocks[i].data = (uint8_t*)malloc(_memoryBlocks[i].size);
        if(_memoryBlocks[i].data == NULL)
            fatalError();

        //read data
        for (uint32_t e=0;e<_memoryBlocks[i].size;e++){
            _memoryBlocks[i].data[e] = EEPROM.read(addr++);
        }
    }
}
uint8_t* SamdPlatform::reloadNVMemory(uint32_t ID, bool pointerAccess)
{
   if(!_MemoryInitialized)
       initNVMemory();

   _MemoryInitialized=true;

   int i;
   for(i=0;i<MAX_MEMORY_BLOCKS;i++){
       if(_memoryBlocks[i].ID == ID)
           break;
   }
   if(i >= MAX_MEMORY_BLOCKS)
       return 0;


   return _memoryBlocks[i].data;
}

void SamdPlatform::finishNVMemory()
{
    uint32_t addr = 0;

    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        if(_memoryBlocks[i].ID == 0)
            continue;

        //write valid mask
        EEPROM.write(addr++,0xBA);
        EEPROM.write(addr++,0xAD);
        EEPROM.write(addr++,0xC0);
        EEPROM.write(addr++,0xDE);

        //write ID
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[0]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[1]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[2]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[3]);

        //write size
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[0]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[1]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[2]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[3]);

        //write data
        for (uint32_t e=0;e<_memoryBlocks[i].size;e++){
            EEPROM.write(addr++,_memoryBlocks[i].data[e]);
        }
    }
    EEPROM.commit();
}

void SamdPlatform::freeNVMemory(uint32_t ID)
{
    int i;
    for(i=0;i<MAX_MEMORY_BLOCKS;i++){
        if(_memoryBlocks[i].ID == ID)
            break;
    }
    if(i >= MAX_MEMORY_BLOCKS)
        return;

    free(_memoryBlocks[i].data);
    _memoryBlocks[i].data = NULL;
    _memoryBlocks[i].size = 0;
    _memoryBlocks[i].ID = 0;
}


uint8_t* SamdPlatform::referenceNVMemory()
{
    return (uint8_t*)0x20000000;       //ram base address
}

#endif

#ifdef RAM_EMULATED_MEMORY
bool SamdPlatform::writeNVMemory(uint8_t* addr,uint8_t data)
{
    *addr = data;
    return true;
}

uint8_t SamdPlatform::readNVMemory(uint8_t* addr)
{
    return *addr;
}

uint8_t* SamdPlatform::allocNVMemory(size_t size,uint32_t ID)
{
    int i;
    for(i=0;i<MAX_MEMORY_BLOCKS;i++){
        if(_memoryBlocks[i].ID == 0)
            break;
    }
    if(i >= MAX_MEMORY_BLOCKS)
        fatalError();


    _memoryBlocks[i].data = (uint8_t*)malloc(size);
    if(_memoryBlocks[i].data == NULL)
        fatalError();

    _memoryBlocks[i].ID = ID;
    _memoryBlocks[i].size = size;

    return _memoryBlocks[i].data;
}

void SamdPlatform::initNVMemory()
{
    uint32_t addr = 0;
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++){

        if (EEPROM.read(addr++) != 0xBA || EEPROM.read(addr++) != 0xAD || EEPROM.read(addr++) != 0xC0 || EEPROM.read(addr++) != 0xDE){
            _memoryBlocks[i].ID = 0;
            _memoryBlocks[i].size = 0;
            _memoryBlocks[i].data = NULL;
            continue;
        }

        ((uint8_t*)&_memoryBlocks[i].ID)[0] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[1] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[2] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].ID)[3] = EEPROM.read(addr++);

        ((uint8_t*)&_memoryBlocks[i].size)[0] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[1] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[2] = EEPROM.read(addr++);
        ((uint8_t*)&_memoryBlocks[i].size)[3] = EEPROM.read(addr++);

        _memoryBlocks[i].data = EEPROM.getDataPtr() + addr;
        addr += _memoryBlocks[i].size;
    }
}

uint8_t* SamdPlatform::reloadNVMemory(uint32_t ID, bool pointerAccess)
{
   if(!_MemoryInitialized)
       initNVMemory();

   _MemoryInitialized=true;

   int i;
   for(i=0;i<MAX_MEMORY_BLOCKS;i++){
       if(_memoryBlocks[i].ID == ID)
           break;
   }
   if(i >= MAX_MEMORY_BLOCKS)
       return 0;


   return _memoryBlocks[i].data;
}

void SamdPlatform::finishNVMemory()
{
    uint32_t addr = 0;

    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        if(_memoryBlocks[i].ID == 0)
            continue;

        //write valid mask
        EEPROM.write(addr++,0xBA);
        EEPROM.write(addr++,0xAD);
        EEPROM.write(addr++,0xC0);
        EEPROM.write(addr++,0xDE);

        //write ID
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[0]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[1]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[2]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].ID)[3]);

        //write size
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[0]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[1]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[2]);
        EEPROM.write(addr++,((uint8_t*)&_memoryBlocks[i].size)[3]);

        //write data
        for (uint32_t e=0;e<_memoryBlocks[i].size;e++){
            EEPROM.write(addr++,_memoryBlocks[i].data[e]);
        }
    }
    EEPROM.commit();
}

void SamdPlatform::freeNVMemory(uint32_t ID)
{
    int i;
    for(i=0;i<MAX_MEMORY_BLOCKS;i++){
        if(_memoryBlocks[i].ID == ID)
            break;
    }
    if(i >= MAX_MEMORY_BLOCKS)
        return;

    _memoryBlocks[i].data = NULL;
    _memoryBlocks[i].size = 0;
    _memoryBlocks[i].ID = 0;
}


uint8_t* SamdPlatform::referenceNVMemory()
{
    return (uint8_t*)0x20000000;       //ram base address
}

#endif

#endif


