#include "memory.h"

#define BASE_ID     0xC0DE0000

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

void Memory::readMemory()
{
    for (int i = 0; i < _saveCount; i++)
    {
        bool pointerAccess;
        if(i<=2)
            pointerAccess = true;
        else
            pointerAccess = false;
        uint8_t* data = _platform.reloadNVMemory(BASE_ID+i, pointerAccess);
        if(data == NULL)
            continue;

        _saveRestores[i]->restore(data);
    }
}

void Memory::writeMemory()
{
    for (int i = 0; i < _saveCount; i++){
        _saveRestores[i]->save();
    }

    _platform.finishNVMemory();
    _modified = false;
}

void Memory::addSaveRestore(SaveRestore * obj)
{
    if (_saveCount >= MAXSAVE - 1)
        return;

    obj->memoryID(BASE_ID + _saveCount);
    _saveRestores[_saveCount] = obj;
    _saveCount += 1;
}
