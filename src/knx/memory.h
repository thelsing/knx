#pragma once

#include <stdint.h>
#include "save_restore.h"
#include "platform.h"
#include "device_object.h"

#define MAXSAVE 10

typedef struct _memoryBlock
{
    uint8_t* address;
    size_t size;
    struct _memoryBlock* next;
} MemoryBlock;

class Memory
{
public:
    Memory(Platform& platform, DeviceObject& deviceObject);
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
    void removeFromFreeList(MemoryBlock* block);
    void addToUsedList(MemoryBlock* block);
    void removeFromUsedList(MemoryBlock* block);
    void addToFreeList(MemoryBlock* block);
    MemoryBlock* removeFromList(MemoryBlock* head, MemoryBlock* item);

    Platform& _platform;
    DeviceObject& _deviceObject;
    bool _modified = false;
    SaveRestore* _saveRestores[MAXSAVE] = {0};
    int _saveCount = 0;
    uint8_t* _data = 0;
    MemoryBlock* _freeList = 0;
    MemoryBlock* _usedList = 0;
};
