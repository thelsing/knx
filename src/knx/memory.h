#pragma once

#include <stdint.h>
#include "save_restore.h"
#include "platform.h"
#include "device_object.h"
#include "table_object.h"

#define MAXSAVE 5
#define MAXTABLEOBJ 4

#ifndef KNX_FLASH_SIZE
#define KNX_FLASH_SIZE 1024
#endif

class MemoryBlock
{
  public:
    MemoryBlock(){};
    MemoryBlock(uint8_t* address, size_t size)
        : address(address), size(size) {}
    uint8_t* address = nullptr;
    size_t size = 0;
    MemoryBlock* next = nullptr;
};

class Memory
{
public:
    Memory(Platform& platform, DeviceObject& deviceObject);
    virtual ~Memory();
    void readMemory();
    void writeMemory();
    void saveMemory();
    void addSaveRestore(SaveRestore* obj);
    void addSaveRestore(TableObject* obj);

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
    uint16_t alignToPageSize(size_t size);
    MemoryBlock* removeFromList(MemoryBlock* head, MemoryBlock* item);
    MemoryBlock* findBlockInList(MemoryBlock* head, uint8_t* address);
    void addNewUsedBlock(uint8_t* address, size_t size);

    void readEraseBlockToBuffer(uint32_t blockNum);
    uint8_t* eraseBlockStart(uint32_t blockNum);
    uint8_t* eraseBlockEnd(uint32_t blockNum);
    void saveBufferdEraseBlock();

    Platform& _platform;
    DeviceObject& _deviceObject;
    SaveRestore* _saveRestores[MAXSAVE] = {0};
    TableObject* _tableObjects[MAXTABLEOBJ] = {0};
    uint8_t _saveCount = 0;
    uint8_t _tableObjCount = 0;
    MemoryBlock* _freeList = nullptr;
    MemoryBlock* _usedList = nullptr;
    uint16_t _metadataSize = 4 + LEN_HARDWARE_TYPE; // accounting for 2x pushWord and pushByteArray of length LEN_HARDWARE_TYPE
};
