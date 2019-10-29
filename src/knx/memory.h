#pragma once

#include <stdint.h>
#include "save_restore.h"
#include "platform.h"

#define MAXSAVE 10

class Memory
{
public:
    Memory(Platform& platform);
    void memoryModified();
    bool isMemoryModified();
    void readMemory();
    void writeMemory();
    void addSaveRestore(SaveRestore* obj);

    uint8_t* allocMemory(size_t size);
    void freeMemory(uint8_t* ptr);
    void writeMemory(uint32_t relativeAddress, size_t size, uint8_t* data);
    uint8_t* toAbsolute(uint32_t relativeAddress);
    uint32_t toRelative(uint8_t* absoluteAddress);

  private:
    Platform& _platform;
    bool _modified = false;
    Restore* _saveRestores[MAXSAVE] = {0};
    int _saveCount = 0;
    uint8_t* _data = 0;
};
