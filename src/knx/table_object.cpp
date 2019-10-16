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

void TableObject::save()
{
    if(_data == NULL)
        return;

    uint8_t* addr =_data - METADATA_SIZE;

    _platform.pushNVMemoryByte(_state, &addr);
    _platform.pushNVMemoryByte(_errorCode, &addr);
    _platform.pushNVMemoryInt(_size, &addr);
}

void TableObject::restore(uint8_t* startAddr)
{
    uint8_t* addr = startAddr;
    _state = (LoadState)_platform.popNVMemoryByte(&addr);
    _errorCode = (ErrorCode)_platform.popNVMemoryByte(&addr);
    _size = _platform.popNVMemoryInt(&addr);

    if (_size > 0)
        _data = addr;
    else
        _data = 0;
}

uint32_t TableObject::tableReference()
{
    return (uint32_t)(_data - _platform.referenceNVMemory());
}

bool TableObject::allocTable(uint32_t size, bool doFill, uint8_t fillByte)
{
    if (_data)
    {
        _platform.freeNVMemory(_ID);
        _data = 0;
        _size = 0;
    }

    if (size == 0)
        return true;
    
    _data = _platform.allocNVMemory(size+this->size(), _ID);
    _data = _data + this->size();  //skip metadata
    _size = size;
    if (doFill){
        uint8_t* addr = _data;
        for(size_t i=0; i<_size;i++)
            _platform.writeNVMemory(addr++, fillByte);
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
