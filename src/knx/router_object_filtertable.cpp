#include "config.h"

#include <cstring>
#include "router_object_filtertable.h"
#include "bits.h"
#include "memory.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

RouterObjectFilterTable::RouterObjectFilterTable(Memory& memory)
    : _memory(memory)
{
    Property* properties[] =
    {
        new CallbackProperty<RouterObjectFilterTable>(this, PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3,
            // ReadCallback of PID_LOAD_STATE_CONTROL
            [](RouterObjectFilterTable* obj, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t {
                if (start == 0)
                    return 1;

                data[0] = obj->_state;
                return 1;
            },
            // WriteCallback of PID_LOAD_STATE_CONTROL
            [](RouterObjectFilterTable* obj, uint16_t start, uint8_t count, const uint8_t* data) -> uint8_t {
                obj->loadEvent(data);
                return 1;
            }),
        new CallbackProperty<RouterObjectFilterTable>(this, PID_TABLE_REFERENCE, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0,
            [](RouterObjectFilterTable* obj, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t {
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                if (obj->_state == LS_UNLOADED)
                    pushInt(0, data);
                else
                    pushInt(obj->tableReference(), data);
                return 1;
            }),

        new DataProperty( PID_MCB_TABLE, false, PDT_GENERIC_08, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // TODO

        new FunctionProperty<RouterObjectFilterTable>(this, PID_ROUTETABLE_CONTROL,
            // Command Callback of PID_ROUTETABLE_CONTROL
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                if (length != 3)
                {
                    resultData[0] = ReturnCodes::DataVoid;
                    resultLength = 1;
                    return;
                }
                uint8_t id = data[1];
                uint8_t info = data[2];
                if (id == 0 && info == 0)
                {
                    //obj->clearFailureLog();
                    resultData[0] = ReturnCodes::Success;
                    resultData[1] = id;
                    resultLength = 2;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            },
            // State Callback of PID_ROUTETABLE_CONTROL
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                if (length != 3)
                {
                    resultData[0] = ReturnCodes::DataVoid;
                    resultLength = 1;
                    return;
                }
                uint8_t id = data[1];
                uint8_t info = data[2];

                // failure counters
                if (id == 0 && info == 0)
                {
                    resultData[0] = ReturnCodes::Success;
                    resultData[1] = id;
                    resultData[2] = info;
                    //obj->getFailureCounters(&resultData[3]); // Put 8 bytes in the buffer
                    resultLength = 3 + 8;
                    return;
                }
                // query latest failure by index
                else if(id == 1)
                {
                    uint8_t maxBufferSize = resultLength; // Remember the maximum buffer size of the buffer that is provided to us
                    uint8_t index = info;
                    uint8_t numBytes = 0;//obj->getFromFailureLogByIndex(index, &resultData[2], maxBufferSize);
                    if ( numBytes > 0)
                    {
                        resultData[0] = ReturnCodes::Success;
                        resultData[1] = id;
                        resultData[2] = index;
                        resultLength += numBytes;
                        resultLength = 3 + numBytes;
                        return;
                    }
                    resultData[0] = ReturnCodes::DataVoid;
                    resultData[1] = id;
                    resultLength = 2;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            }),

        new DataProperty( PID_FILTER_TABLE_USE, false, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // TODO
/*
        new FunctionProperty<RouterObjectFilterTable>(this, PID_RF_ENABLE_SBC,
            // Command Callback of PID_RF_ENABLE_SBC
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                uint8_t serviceId = data[1] & 0xff;
                if (serviceId != 0)
                {
                    resultData[0] = ReturnCodes::InvalidCommand;
                    resultLength = 1;
                    return;
                }
                if (length == 3)
                {
                    uint8_t mode = data[2];
                    if (mode > 1)
                    {
                        resultData[0] = ReturnCodes::DataVoid;
                        resultLength = 1;
                        return;
                    }
                    //obj->setSecurityMode(mode == 1);
                    resultData[0] = ReturnCodes::Success;
                    resultData[1] = serviceId;
                    resultLength = 2;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            },
            // State Callback of PID_RF_ENABLE_SBC
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                uint8_t serviceId = data[1] & 0xff;
                if (serviceId != 0)
                {
                    resultData[0] = ReturnCodes::InvalidCommand;
                    resultLength = 1;
                    return;
                }
                if (length == 2)
                {
                    resultData[0] = ReturnCodes::Success;
                    resultData[1] = serviceId;
                    resultData[2] = 0;//obj->isSecurityModeEnabled() ? 1 : 0;
                    resultLength = 3;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            }),
*/
    };

    RouterObject::initializeProperties(sizeof(properties), properties);
}

uint8_t* RouterObjectFilterTable::save(uint8_t* buffer)
{
    buffer = pushByte(_state, buffer);

    if (_data)
        buffer = pushInt(_memory.toRelative(_data), buffer);
    else
        buffer = pushInt(0, buffer);

    return RouterObject::save(buffer);
}

const uint8_t* RouterObjectFilterTable::restore(const uint8_t* buffer)
{
    uint8_t state = 0;
    buffer = popByte(state, buffer);
    _state = (LoadState)state;

    uint32_t relativeAddress = 0;
    buffer = popInt(relativeAddress, buffer);

    if (relativeAddress != 0)
        _data = _memory.toAbsolute(relativeAddress);
    else
        _data = 0;

    return RouterObject::restore(buffer);
}

uint16_t RouterObjectFilterTable::saveSize()
{
    return 1 + 4 + RouterObject::saveSize();
}

uint32_t RouterObjectFilterTable::tableReference()
{
    return (uint32_t)_memory.toRelative(_data);
}

bool RouterObjectFilterTable::isLoaded()
{
    return _state == LS_LOADED;
}

LoadState RouterObjectFilterTable::loadState()
{
    return _state;
}

void RouterObjectFilterTable::loadEvent(const uint8_t* data)
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

void RouterObjectFilterTable::loadEventUnloaded(const uint8_t* data)
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
            errorCode(E_GOT_UNDEF_LOAD_CMD);
    }
}

void RouterObjectFilterTable::loadEventLoading(const uint8_t* data)
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
        case LE_ADDITIONAL_LOAD_CONTROLS: // Not supported here
        default:
            loadState(LS_ERROR);
            errorCode(E_GOT_UNDEF_LOAD_CMD);
    }
}

void RouterObjectFilterTable::loadEventLoaded(const uint8_t* data)
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
            errorCode(E_INVALID_OPCODE);
            break;
        default:
            loadState(LS_ERROR);
            errorCode(E_GOT_UNDEF_LOAD_CMD);
    }
}

void RouterObjectFilterTable::loadEventError(const uint8_t* data)
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
            errorCode(E_GOT_UNDEF_LOAD_CMD);
    }
}

void RouterObjectFilterTable::loadState(LoadState newState)
{
    if (newState == _state)
        return;
    //beforeStateChange(newState);
    _state = newState;
}

void RouterObjectFilterTable::errorCode(ErrorCode errorCode)
{
    uint8_t data = errorCode;
    Property* prop = property(PID_ERROR_CODE);
    prop->write(data);
}

void RouterObjectFilterTable::masterReset(EraseCode eraseCode, uint8_t channel)
{
    RouterObject::masterReset(eraseCode, channel);

    if (eraseCode == FactoryReset)
    {
        // TODO handle different erase codes
        println("Factory reset of router object with filter table requested.");
    }
}
