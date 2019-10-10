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

void Memory::readMemory(){
    switch (_platform.NVMemoryType()){
    case internalRam:
        readRamMemory();
        break;
    case internalFlash:
        readFlashMemory();
        break;
    case external:
        readExternalMemory();
        break;
    case notDefined:
    default:
        _platform.fatalError();
    }
}

void Memory::writeMemory(){
    switch (_platform.NVMemoryType()){
    case internalRam:
        writeRamMemory();
        break;
    case internalFlash:
        writeFlashMemory();
        break;
    case external:
        writeExternalMemory();
        break;
    case notDefined:
    default:
        _platform.fatalError();
    }
}

void Memory::readFlashMemory()
{
    for (int i = 0; i < _saveCount; i++)
    {
        uint8_t* data = (uint8_t*)_platform.reloadNVMemory(BASE_ID+i);
        if(data == NULL)
            continue;

        _saveRestores[i]->restore(data);

    }
}

void Memory::writeFlashMemory()
{
    for (int i = 0; i < _saveCount; i++){
        _saveRestores[i]->save();
    }

    _platform.finishNVMemory();
    _modified = false;
}

void Memory::readRamMemory()
{
    _data = (uint8_t*)_platform.reloadNVMemory(1);

    if (_data[0] != 0x00 || _data[1] != 0xAD || _data[2] != 0xAF || _data[3] != 0xFE)
        return;

    uint8_t* buffer = _data + 4;
    int size = _saveCount;
    for (int i = 0; i < size; i++)
    {
        buffer = _saveRestores[i]->restore(buffer);
        buffer = (uint8_t*)(((uintptr_t)buffer + 3) / 4 * 4);  //allign to 32bit
    }
}

void Memory::writeRamMemory()
{
    uint32_t bytesToSave = 10;

    for (int i = 0; i < _saveCount; i++){
        bytesToSave += _saveRestores[i]->size();
    }

    _data = (uint8_t*)_platform.allocNVMemory(bytesToSave,BASE_ID);

    _data[0] = 0x00;
    _data[1] = 0xAD;
    _data[2] = 0xAF;
    _data[3] = 0xFE;

    uint8_t* buffer = _data + 4;
    int size = _saveCount;
    for (int i = 0; i < size; i++)
    {
        buffer = _saveRestores[i]->save(buffer);
        buffer = (uint8_t*)(((uintptr_t)buffer + 3) / 4 * 4);  //allign to 32bit
    }
    _platform.finishNVMemory();
    _modified = false;
}




void Memory::readExternalMemory()
{

    int size = _saveCount;
    volatile uintptr_t addr = _platform.reloadNVMemory(BASE_ID);
    volatile uint32_t bytesToRestore;

    if(addr == 0)
        return;


    if (_platform.readNVMemory(addr++) != 0x00 || _platform.readNVMemory(addr++) != 0xAD || _platform.readNVMemory(addr++) != 0xAF || _platform.readNVMemory(addr++) != 0xFE)
        return;


    for (int i = 0; i < size; i++)
    {
        ((uint8_t*)&bytesToRestore)[0] = _platform.readNVMemory(addr++);
        ((uint8_t*)&bytesToRestore)[1] = _platform.readNVMemory(addr++);
        ((uint8_t*)&bytesToRestore)[2] = _platform.readNVMemory(addr++);
        ((uint8_t*)&bytesToRestore)[3] = _platform.readNVMemory(addr++);

        if(bytesToRestore == 0)
            continue;
        _data = _platform.allocMemory(bytesToRestore);
        if(_data == NULL)
            _platform.fatalError();

        for (uint32_t e=0;e<bytesToRestore;e++){
            _data[e] = _platform.readNVMemory(addr++);
        }
        _saveRestores[i]->restore(_data);
    }
}

void Memory::writeExternalMemory()
{
    uint32_t bytesToSave = 4;
    int size = _saveCount;

    _platform.freeNVMemory(BASE_ID);

    for (int i = 0; i < size; i++){
        bytesToSave += _saveRestores[i]->size() + 4;
    }

    uintptr_t addr = _platform.allocNVMemory(bytesToSave,BASE_ID);

    //write valid mask
    _platform.writeNVMemory(addr++,0x00);
    _platform.writeNVMemory(addr++,0xAD);
    _platform.writeNVMemory(addr++,0xAF);
    _platform.writeNVMemory(addr++,0xFE);

    for (int i = 0; i < size; i++)
    {
        _data = _saveRestores[i]->save();
        if(_data == NULL)
            bytesToSave = 0;
        else
            bytesToSave = _saveRestores[i]->size();

        //write size
        _platform.writeNVMemory(addr++,((uint8_t*)&bytesToSave)[0]);
        _platform.writeNVMemory(addr++,((uint8_t*)&bytesToSave)[1]);
        _platform.writeNVMemory(addr++,((uint8_t*)&bytesToSave)[2]);
        _platform.writeNVMemory(addr++,((uint8_t*)&bytesToSave)[3]);

        if(bytesToSave == 0)
            continue;


        for (uint32_t e=0;e<bytesToSave;e++){
            _platform.writeNVMemory(addr++,_data[e]);
        }

        _platform.freeMemory(_data);
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
