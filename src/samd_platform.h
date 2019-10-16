#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_SAMD

//define which memory type is used for non-volatile memory
#define INTERN_FLASH_MEMORY
//#define EXTERN_EEPROM_MEMORY
//#define RAM_EMULATED_MEMORY       //like FlashStorage lib

#ifndef INTERN_FLASH_MEMORY
#define MAX_MEMORY_BLOCKS   6

typedef struct{
    uint32_t ID;
    size_t  size;
    uint8_t* data;
}MemoryBlock_t;
#endif

class SamdPlatform : public ArduinoPlatform
{
public:
    SamdPlatform();
    SamdPlatform( HardwareSerial* s);

    void restart();
    bool writeNVMemory(uint8_t* addr,uint8_t data);
    uint8_t readNVMemory(uint8_t* addr);
    uint8_t* allocNVMemory(size_t size,uint32_t ID);
    uint8_t* reloadNVMemory(uint32_t ID, bool pointerAccess);
    void finishNVMemory();
    void freeNVMemory(uint32_t ID);
    uint8_t* referenceNVMemory();
private:
#ifndef INTERN_FLASH_MEMORY
    void initNVMemory();
    MemoryBlock_t _memoryBlocks[MAX_MEMORY_BLOCKS];
    bool _MemoryInitialized = false;
#endif
};

#endif
