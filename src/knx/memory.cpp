#include "memory.h"
#include <string.h>

Memory::Memory(Platform & platform): _platform(platform)
{
}

void Memory::memoryModified()
{
    _modified = true;
}

bool Memory::isMemoryModified()
{
    return _modified;
}

// TODO implement flash layout: manufacturerID, HarwareType, Version, addr[0], size[0], addr[1], size[1], ...
// reconstruct free flash list and used list on read
void Memory::readMemory()
{
    _data = _platform.getEepromBuffer(512);

    if (_data[0] != 0x00 || _data[1] != 0xAD || _data[2] != 0xAF || _data[3] != 0xFE)
        return;

    uint8_t* buffer = _data + 4;
    int size = _saveCount;
    for (int i = 0; i < size; i++)
    {
        buffer = _saveRestores[i]->restore(buffer);
    }
}

void Memory::writeMemory()
{
    _data[0] = 0x00;
    _data[1] = 0xAD;
    _data[2] = 0xAF;
    _data[3] = 0xFE;

    uint8_t* buffer = _data + 4;
    int size = _saveCount;
    for (int i = 0; i < size; i++)
    {
        SaveRestore* saveRestore = dynamic_cast<SaveRestore*>(_saveRestores[i]);
        if(saveRestore)
            buffer = saveRestore->save(buffer);
    }
    _platform.commitToEeprom();
    _modified = false;
}

void Memory::addSaveRestore(SaveRestore * obj)
{
    if (_saveCount >= MAXSAVE - 1)
        return;

    _saveRestores[_saveCount] = obj;
    _saveCount += 1;
}


uint8_t* Memory::allocMemory(size_t size)
{
    return _platform.allocMemory(size);
}


void Memory::freeMemory(uint8_t* ptr)
{
    return _platform.freeMemory(ptr);
}

void Memory::writeMemory(uint32_t relativeAddress, size_t size, uint8_t* data)
{
    memcpy(toAbsolute(relativeAddress), data, size);
    memoryModified();
}


uint8_t* Memory::toAbsolute(uint32_t relativeAddress)
{
    return _platform.memoryReference() + (ptrdiff_t)relativeAddress;
}


uint32_t Memory::toRelative(uint8_t* absoluteAddress)
{
    return absoluteAddress - _platform.memoryReference();
}
