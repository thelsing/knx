#include <string.h>

#include "table_object.h"
#include "bits.h"
#include "memory.h"
#include "callback_property.h"
#include "data_property.h"

BeforeTablesUnloadCallback TableObject::_beforeTablesUnload = 0;
uint8_t TableObject::_tableUnloadCount = 0;

void TableObject::beforeTablesUnloadCallback(BeforeTablesUnloadCallback func)
{
    _beforeTablesUnload = func;
}

BeforeTablesUnloadCallback TableObject::beforeTablesUnloadCallback()
{
    return _beforeTablesUnload;
}

TableObject::TableObject(Memory& memory, uint32_t staticTableAdr , uint32_t staticTableSize)
    : _memory(memory)
{
    _staticTableAdr = staticTableAdr;
    _staticTableSize = staticTableSize;
}

TableObject::~TableObject()
{}

void TableObject::beforeStateChange(LoadState& newState)
{
    if (newState == LS_LOADED && _tableUnloadCount > 0)
        _tableUnloadCount--;
    if (_tableUnloadCount > 0)
        return;
    if (newState == LS_UNLOADED) {
        _tableUnloadCount++;
        if (_beforeTablesUnload != 0)
            _beforeTablesUnload();
    }
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
    //println("TableObject::save");
    allocTableStatic();

    buffer = pushByte(_state, buffer);

    buffer = pushInt(_size, buffer);

    if (_data)
        buffer = pushInt(_memory.toRelative(_data), buffer);
    else
        buffer = pushInt(0, buffer);

    return InterfaceObject::save(buffer);
}


const uint8_t* TableObject::restore(const uint8_t* buffer)
{
    //println("TableObject::restore");

    uint8_t state = 0;
    buffer = popByte(state, buffer);
    _state = (LoadState)state;

    buffer = popInt(_size, buffer);

    uint32_t relativeAddress = 0;
    buffer = popInt(relativeAddress, buffer);
    //println(relativeAddress);

    if (relativeAddress != 0)
        _data = _memory.toAbsolute(relativeAddress);
    else
        _data = 0;
    //println((uint32_t)_data);
    return InterfaceObject::restore(buffer);
}

uint32_t TableObject::tableReference()
{
    return (uint32_t)_memory.toRelative(_data);
}

bool TableObject::allocTable(uint32_t size, bool doFill, uint8_t fillByte)
{
    if(_staticTableAdr)
        return false;

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
    {
        uint32_t addr = _memory.toRelative(_data);
        for(uint32_t i = 0; i < size;i++)
            _memory.writeMemory(addr+i, 1, &fillByte);
    }

    _size = size;

    return true;
}


void TableObject::allocTableStatic()
{
    if(_staticTableAdr && !_data)
    {
        _data = _memory.toAbsolute(_staticTableAdr);
        _size = _staticTableSize;
        _memory.addNewUsedBlock(_data, _size);
    }
}

void TableObject::loadEvent(const uint8_t* data)
{
    //printHex("TableObject::loadEvent 0x", data, 10);
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

void TableObject::loadEventUnloaded(const uint8_t* data)
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

void TableObject::loadEventLoading(const uint8_t* data)
{
    uint8_t event = data[0];
    switch (event)
    {
        case LE_NOOP:
        case LE_START_LOADING:
            break;
        case LE_LOAD_COMPLETED:
            _memory.saveMemory();
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

void TableObject::loadEventLoaded(const uint8_t* data)
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
                if(!_staticTableAdr)
                {
                    _memory.freeMemory(_data);
                    _data = 0;
                }
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

void TableObject::loadEventError(const uint8_t* data)
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

void TableObject::additionalLoadControls(const uint8_t* data)
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

uint8_t* TableObject::data()
{
    return _data;
}

void TableObject::errorCode(ErrorCode errorCode)
{
    uint8_t data = errorCode;
    Property* prop = property(PID_ERROR_CODE);
    prop->write(data);
}

uint16_t TableObject::saveSize()
{
    return 5 + InterfaceObject::saveSize() + sizeof(_size);
}

void TableObject::initializeProperties(size_t propertiesSize, Property** properties)
{
    Property* ownProperties[] =
    {
        new CallbackProperty<TableObject>(this, PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3,
            [](TableObject* obj, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t {
                if(start == 0)
                {
                    uint16_t currentNoOfElements = 1;
                    pushWord(currentNoOfElements, data);
                    return 1;
                }

                data[0] = obj->_state;
                return 1;
            },
            [](TableObject* obj, uint16_t start, uint8_t count, const uint8_t* data) -> uint8_t {
                obj->loadEvent(data);
                return 1;
            })
     };

    uint8_t ownPropertiesCount = sizeof(ownProperties) / sizeof(Property*);

    uint8_t propertyCount = propertiesSize / sizeof(Property*);
    uint8_t allPropertiesCount = propertyCount + ownPropertiesCount;

    Property* allProperties[allPropertiesCount];
    memcpy(allProperties, properties, propertiesSize);
    memcpy(allProperties + propertyCount, ownProperties, sizeof(ownProperties));

    if(_staticTableAdr)
        InterfaceObject::initializeProperties(sizeof(allProperties), allProperties);
    else
        initializeDynTableProperties(sizeof(allProperties), allProperties);
}

void TableObject::initializeDynTableProperties(size_t propertiesSize, Property** properties)
{
    Property* ownProperties[] =
    {
        new CallbackProperty<TableObject>(this, PID_TABLE_REFERENCE, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0,
            [](TableObject* obj, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t {
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
        new CallbackProperty<TableObject>(this, PID_MCB_TABLE, false, PDT_GENERIC_08, 1, ReadLv3 | WriteLv0,
            [](TableObject* obj, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t {
                if (obj->_state != LS_LOADED)
                    return 0; // need to check return code for invalid
                
                uint32_t segmentSize = obj->_size;
                uint16_t crc16 = crc16Ccitt(obj->data(), segmentSize); 

                pushInt(segmentSize, data);     // Segment size
                pushByte(0x00, data + 4);       // CRC control byte -> 0: always valid
                pushByte(0xFF, data + 5);       // Read access 4 bits + Write access 4 bits
                pushWord(crc16, data + 6);      // CRC-16 CCITT of data
    
                return 1;
            }),
        new DataProperty(PID_ERROR_CODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint8_t)E_NO_FAULT)
     };

    uint8_t ownPropertiesCount = sizeof(ownProperties) / sizeof(Property*);

    uint8_t propertyCount = propertiesSize / sizeof(Property*);
    uint8_t allPropertiesCount = propertyCount + ownPropertiesCount;

    Property* allProperties[allPropertiesCount];
    memcpy(allProperties, properties, propertiesSize);
    memcpy(allProperties + propertyCount, ownProperties, sizeof(ownProperties));

    InterfaceObject::initializeProperties(sizeof(allProperties), allProperties);
}