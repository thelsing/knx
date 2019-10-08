#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_SAMD

class SamdPlatform : public ArduinoPlatform
{
public:
    SamdPlatform();
    SamdPlatform( HardwareSerial* s);

    void restart();
    bool writeNVMemory(uint32_t addr,uint8_t data);
    uint8_t readNVMemory(uint32_t addr);
    uint32_t allocNVMemory(uint32_t size,uint32_t ID);
    uint32_t reloadNVMemory(uint32_t ID);
    void finishNVMemory();
    void freeNVMemory(uint32_t ID);
    uint8_t* memoryReference();
};

#endif
