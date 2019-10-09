#include <string.h>

#include "table_object.h"
#include "bits.h"

#define METADATA_SIZE     (sizeof(_state)+sizeof(_errorCode)+sizeof(_size))

TableObject::TableObject(Platform& platform): _platform(platform)
{

}

void TableObject::readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (id)
    {
        case PID_LOAD_STATE_CONTROL:
            data[0] = _state;
            break;
        case PID_TABLE_REFERENCE:
            if (_state == LS_UNLOADED)
                pushInt(0, data);
            else
                pushInt(tableReference(), data);
            break;
        case PID_ERROR_CODE:
            data[0] = _errorCode;
            break;
        default:
            InterfaceObject::readProperty(id, start, count, data);
    }
}

void TableObject::writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count)
{
    switch (id)
    {
        case PID_LOAD_STATE_CONTROL:
            loadEvent(data);
            break;

        //case PID_MCB_TABLE:
        //    TODO
        //    break;
        default:
            InterfaceObject::writeProperty(id, start, data, count);
    }
}

uint8_t TableObject::propertySize(PropertyID id)
{
    switch (id)
    {
        case PID_LOAD_STATE_CONTROL:
            return 1;
        case PID_TABLE_REFERENCE:
            return 4;
        case PID_ERROR_CODE:
            return 1;
        case PID_OBJECT_TYPE:
            return 2;
        default:
            return InterfaceObject::propertySize(id);
    }
}

TableObject::~TableObject()
{
    if (_data != 0)
        _platform.freeMemory(_data);
}

LoadState TableObject::loadState()
{
    return _state;
}

void TableObject::loadState(LoadState newState)
{
    if (newState == _state)
        return;
    beforeStateChange(newState);
    _state = newState;
}


uint8_t* TableObject::save(uint8_t* buffer)
{
    buffer = pushByte(_state, buffer);
    buffer = pushByte(_errorCode, buffer);
    buffer = pushInt(_size, buffer);
    buffer = pushByteArray(_data, _size, buffer);

    return buffer;
}

uint8_t* TableObject::save()
{
    if(_data == NULL)
        return NULL;

    uintptr_t addr;
    uint8_t* buffer;
    addr =(uintptr_t)(_data - METADATA_SIZE);
    if(_platform.NVMemoryType() == internalFlash)
        buffer = new uint8_t[METADATA_SIZE];
    else
        buffer = (uint8_t*)addr;

    buffer = pushByte(_state, buffer);
    buffer = pushByte(_errorCode, buffer);
    buffer = pushInt(_size, buffer);
    buffer -= METADATA_SIZE;

    if(_platform.NVMemoryType() == internalFlash){
        for(uint32_t i=0;i<METADATA_SIZE;i++)
            _platform.writeNVMemory(addr+i, buffer[i]);

        delete[] buffer;
    }

    return _dataComplete;
}

uint8_t* TableObject::restore(uint8_t* buffer)
{
    uint8_t state = 0;
    uint8_t errorCode = 0;
    if(_dataComplete == NULL)
        _dataComplete = buffer;
    buffer = popByte(state, buffer);
    buffer = popByte(errorCode, buffer);
    _state = (LoadState)state;
    _errorCode = (ErrorCode)errorCode;

    buffer = popInt(_size, buffer);

    if (_size > 0)
        _data = buffer;

    else
        _data = 0;

    buffer += _size;

    return buffer;
}

uint32_t TableObject::tableReference()
{
    return (uint32_t)(_data - _platform.memoryReference());
}

bool TableObject::allocTable(uint32_t size, bool doFill, uint8_t fillByte)
{
    if (_dataComplete)
    {
        if(_platform.NVMemoryType() == internalFlash)
            _platform.freeNVMemory(_ID);
        else if(_platform.NVMemoryType() == external)
            _platform.freeMemory(_dataComplete);
        _dataComplete = 0;
        _data = 0;
        _size = 0;
    }

    if (size == 0)
        return true;
    
    if(_platform.NVMemoryType() == internalFlash){
        _dataComplete = (uint8_t*)_platform.allocNVMemory(size+this->size(), _ID);
    }
    else{
        _dataComplete = _platform.allocMemory(size+this->size());
    }
    _data = _dataComplete + this->size();  //skip metadata
    _size = size;
    if (doFill){
        if(_platform.NVMemoryType() == internalFlash){
            uintptr_t addr = (uintptr_t)_data;
            for(size_t i=0; i<_size;i++)
                _platform.writeNVMemory(addr++, fillByte);
        }
        else{
            memset(_data, fillByte, _size);
        }
    }
    return true;
}

void TableObject::loadEvent(uint8_t* data)
{
    switch (_state)
    {
        case LS_UNLOADED:
            loadEventUnloaded(data);
            break;
        case LS_LOADING:
            loadEventLoading(data);
            break;
        case LS_LOADED:
            loadEventLoaded(data);
            break;
        case LS_ERROR:
            loadEventError(data);
            break;
        default:
            /* do nothing */
            break;
    }
}

void TableObject::loadEventUnloaded(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
        case LE_NOOP:
        case LE_LOAD_COMPLETED:
        case LE_ADDITIONAL_LOAD_CONTROLS:
        case LE_UNLOAD:
            break;
        case LE_START_LOADING:
            loadState(LS_LOADING);
            break;
        default:
            loadState(LS_ERROR);
            _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void TableObject::loadEventLoading(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
        case LE_NOOP:
        case LE_START_LOADING:
            break;
        case LE_LOAD_COMPLETED:
            loadState(LS_LOADED);
            break;
        case LE_UNLOAD:
            loadState(LS_UNLOADED);
            break;
        case LE_ADDITIONAL_LOAD_CONTROLS:
            additionalLoadControls(data);
            break;
        default:
            loadState(LS_ERROR);
            _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void TableObject::loadEventLoaded(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
        case LE_NOOP:
        case LE_LOAD_COMPLETED:
            break;
        case LE_START_LOADING:
            loadState(LS_LOADING);
            break;
        case LE_UNLOAD:
            loadState(LS_UNLOADED);
            break;
        case LE_ADDITIONAL_LOAD_CONTROLS:
            loadState(LS_ERROR);
            _errorCode = E_INVALID_OPCODE;
            break;
        default:
            loadState(LS_ERROR);
            _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void TableObject::loadEventError(uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
        case LE_NOOP:
        case LE_LOAD_COMPLETED:
        case LE_ADDITIONAL_LOAD_CONTROLS:
        case LE_START_LOADING:
            break;
        case LE_UNLOAD:
            loadState(LS_UNLOADED);
            break;
        default:
            loadState(LS_ERROR);
            _errorCode = E_GOT_UNDEF_LOAD_CMD;
    }
}

void TableObject::additionalLoadControls(uint8_t* data)
{
    if (data[1] != 0x0B) // Data Relative Allocation
    {
        loadState(LS_ERROR);
        _errorCode = E_INVALID_OPCODE;
        return;
    }

    size_t size = ((data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5]);
    bool doFill = data[6] == 0x1;
    uint8_t fillByte = data[7];
    if (!allocTable(size, doFill, fillByte))
    {
        loadState(LS_ERROR);
        _errorCode = E_MAX_TABLE_LENGTH_EXEEDED;
    }
}

uint8_t* TableObject::data()
{
    return _data;
}

uint32_t TableObject::size()
{
    return _size + METADATA_SIZE;
}

uint32_t TableObject::sizeMetadata()
{
    return METADATA_SIZE;
}

void TableObject::errorCode(ErrorCode errorCode)
{
    _errorCode = errorCode;
}
