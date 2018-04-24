#include <stdlib.h>
#include <string.h>

#include "table_object.h"
#include "bits.h"

TableObject::TableObject(uint8_t* memoryReference): _memoryReference(memoryReference)
{

}

void TableObject::readProperty(PropertyID id, uint32_t start, uint32_t count, uint8_t* data)
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
    }
    return 0;
}

TableObject::~TableObject()
{
    if (_data != 0)
        free(_data);
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


uint8_t* TableObject::restore(uint8_t* buffer)
{
    uint8_t state = 0;
    uint8_t errorCode = 0;
    buffer = popByte(state, buffer);
    buffer = popByte(errorCode, buffer);
    _state = (LoadState)state;
    _errorCode = (ErrorCode)errorCode;

    buffer = popInt(_size, buffer);

    if (_data)
        free(_data);

    _data = (uint8_t*) malloc(_size);

    buffer = popByteArray(_data, _size, buffer);

    return buffer;
}

uint32_t TableObject::tableReference()
{
    return (uint32_t)(_data - _memoryReference);
}

bool TableObject::allocTable(uint32_t size, bool doFill, uint8_t fillByte)
{
    if (_data)
    {
        free(_data);
        _size = 0;
    }

    _data = (uint8_t*)malloc(size);
    if (!_data)
        return false;

    _size = size;

    if (doFill)
        memset(_data, fillByte, size);

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