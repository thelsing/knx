#include "memory.h"

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
    //_data = _platform.getEepromBuffer(512);
    //uint8_t* buffer = _data;
    //int size = _saveCount;
    //for (int i = 0; i < size; i++)
    //{
    //    buffer = _saveRestores[i]->restore(buffer);
    //}
}

void Memory::writeMemory()
{
    //uint8_t* buffer = _data;
    //int size = _saveCount;
    //for (int i = 0; i < size; i++)
    //{
    //    buffer = _saveRestores[i]->save(buffer);
    //}
    //_platform.commitToEeprom();
    //_modified = false;
}

void Memory::addSaveRestore(SaveRestore * obj)
{
    if (_saveCount >= MAXSAVE - 1)
        return;

    _saveRestores[_saveCount] = obj;
    _saveCount += 1;
}
