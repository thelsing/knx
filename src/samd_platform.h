#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_SAMD

class SamdPlatform : public ArduinoPlatform
{
public:
    SamdPlatform();
    SamdPlatform( HardwareSerial* s);

    void restart();
    bool writeNVMemory(uintptr_t addr,uint8_t data);
    uint8_t readNVMemory(uintptr_t addr);
    uintptr_t allocNVMemory(size_t size,uint32_t ID);
    uintptr_t reloadNVMemory(uint32_t ID);
    void finishNVMemory();
    void freeNVMemory(uint32_t ID);
    uint8_t* memoryReference();
};

#endif
