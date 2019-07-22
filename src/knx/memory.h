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
private:
    Platform& _platform;
    bool _modified = false;
    SaveRestore* _saveRestores[MAXSAVE] = {0};
    int _saveCount = 0;
    uint8_t* _data = 0;
};
