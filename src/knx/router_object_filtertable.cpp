#include "config.h"

#include <cstring>
#include "router_object_filtertable.h"
#include "bits.h"
#include "memory.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

enum RouteTableServices
{
    ClearRoutingTable = 0x01, // no info bytes
    SetRoutingTable = 0x02,   // no info bytes
    ClearGroupAddress = 0x03, // 4 bytes: start address and end address
    SetGroupAddress = 0x04,   // 4 bytes: start address and end address
};

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

        new DataProperty( PID_MCB_TABLE, false, PDT_GENERIC_08, 1, ReadLv3 | WriteLv0),

        new FunctionProperty<RouterObjectFilterTable>(this, PID_ROUTETABLE_CONTROL,
            // Command Callback of PID_ROUTETABLE_CONTROL
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRouteTableControl(true, data, length, resultData, resultLength);
            },
            // State Callback of PID_ROUTETABLE_CONTROL
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRouteTableControl(false, data, length, resultData, resultLength);
            }),

        new DataProperty( PID_FILTER_TABLE_USE, false, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, (uint16_t) 0 ), // TODO

        new FunctionProperty<RouterObjectFilterTable>(this, PID_RF_ENABLE_SBC,
            // Command Callback of PID_RF_ENABLE_SBC
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRfEnableSbc(true, data, length, resultData, resultLength);
            },
            // State Callback of PID_RF_ENABLE_SBC
            [](RouterObjectFilterTable* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                obj->functionRfEnableSbc(false, data, length, resultData, resultLength);
            }),
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

    _filterTableGroupAddresses = (uint16_t*)_data;

    return RouterObject::restore(buffer);
}

uint16_t RouterObjectFilterTable::saveSize()
{
    return 1 + 4 + RouterObject::saveSize();
}

void RouterObjectFilterTable::functionRouteTableControl(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    bool isError = false;
    RouteTableServices srvId = (RouteTableServices) data[1];

    // Filter Table Realization Type 3
    // The Filter Table Realisation Type 3 shall be organised as a memory mapped bit-field of
    // 65536 bits and thus 8 192 octets. Each bit shall uniquely correspond to one Group Address.
    // The full 16 bit KNX GA encoding range shall be supported.
    //
    // octet_address = GA_value div 8
    // bit_position = GA_value mod 8

    if (isCommand)
    {
        switch(srvId)
        {
            case ClearRoutingTable:
            case SetRoutingTable:
            case ClearGroupAddress:
            case SetGroupAddress: break;
            default: isError = true;
        }
    }
    else
    {
        switch(srvId)
        {
            case ClearRoutingTable:
            case SetRoutingTable:
            case ClearGroupAddress:
            case SetGroupAddress: break;
            default: isError = true;
        }
    }

    if (isError)
    {
        resultData[0] = ReturnCodes::GenericError;
        resultData[1] = srvId;
        resultLength = 2;
    }
}

void RouterObjectFilterTable::functionRfEnableSbc(bool isCommand, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    if (isCommand)
    {
        _rfSbcRoutingEnabled = (data[0] == 1) ? true : false;
    }

    resultData[0] = ReturnCodes::Success;
    resultData[1] = _rfSbcRoutingEnabled ? 1 : 0;
    resultLength = 2;
}

bool RouterObjectFilterTable::isRfSbcRoutingEnabled()
{
    return _rfSbcRoutingEnabled;
}

uint32_t RouterObjectFilterTable::tableReference()
{
    return (uint32_t)_memory.toRelative(_data);
}

bool RouterObjectFilterTable::allocTable(uint32_t size, bool doFill, uint8_t fillByte)
{
    if (_data)
    {
        _memory.freeMemory(_data);
        _data = 0;
    }

    if (size == 0)
        return true;

    _data = _memory.allocMemory(size);
    if (!_data)
        return false;

    if (doFill)
        memset(_data, fillByte, size);

    return true;
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
        case LE_ADDITIONAL_LOAD_CONTROLS:
            additionalLoadControls(data);
            break;
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
            //free nv memory
            if (_data)
            {
                _memory.freeMemory(_data);
                _data = 0;
            }
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

void RouterObjectFilterTable::additionalLoadControls(const uint8_t* data)
{
    if (data[1] != 0x0B) // Data Relative Allocation
    {
        loadState(LS_ERROR);
        errorCode(E_INVALID_OPCODE);
        return;
    }

    size_t size = ((data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5]);
    bool doFill = data[6] == 0x1;
    uint8_t fillByte = data[7];
    if (!allocTable(size, doFill, fillByte))
    {
        loadState(LS_ERROR);
        errorCode(E_MAX_TABLE_LENGTH_EXEEDED);
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

void RouterObjectFilterTable::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    // calculate crc16-ccitt for PID_MCB_TABLE
    updateMcb();

    _filterTableGroupAddresses = (uint16_t*)_data;
}

void RouterObjectFilterTable::updateMcb()
{
    uint8_t mcb[propertySize(PID_MCB_TABLE)];

    static constexpr uint32_t segmentSize = 8192;
    uint16_t crc16 = crc16Ccitt(_data, segmentSize);

    pushInt(segmentSize, &mcb[0]); // Segment size
    pushByte(0x00, &mcb[4]);       // CRC control byte -> 0: always valid -> according to coupler spec. it shall always be a valid CRC
    pushByte(0xFF, &mcb[5]);       // Read access 4 bits + Write access 4 bits (unknown: value taken from real coupler device)
    pushWord(crc16, &mcb[6]);      // CRC-16 CCITT of filter table

    property(PID_MCB_TABLE)->write(mcb);
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
