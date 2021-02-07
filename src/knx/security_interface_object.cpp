#include "config.h"
#ifdef USE_DATASECURE

#include <cstring>
#include "security_interface_object.h"
#include "secure_application_layer.h"
#include "bits.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

// Our FDSK. It is never changed from ETS. This is the permanent default tool key that is restored on every factory reset of the device.
const uint8_t SecurityInterfaceObject::_fdsk[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
uint8_t SecurityInterfaceObject::_secReport[] = { 0x00, 0x00, 0x00 };
uint8_t SecurityInterfaceObject::_secReportCtrl[] = { 0x00, 0x00, 0x00 };

SecurityInterfaceObject::SecurityInterfaceObject()
{
    Property* properties[] =
    {
        new DataProperty( PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_SECURITY ),
        new CallbackProperty<SecurityInterfaceObject>(this, PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3,
            // ReadCallback of PID_LOAD_STATE_CONTROL
            [](SecurityInterfaceObject* obj, uint16_t start, uint8_t count, uint8_t* data) -> uint8_t {
                if (start == 0)
                    return 1;

                data[0] = obj->_state;
                return 1;
            },
            // WriteCallback of PID_LOAD_STATE_CONTROL
            [](SecurityInterfaceObject* obj, uint16_t start, uint8_t count, const uint8_t* data) -> uint8_t {
                obj->loadEvent(data);
                return 1;
            }),
        new FunctionProperty<SecurityInterfaceObject>(this, PID_SECURITY_MODE,
            // Command Callback of PID_SECURITY_MODE
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
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
                    obj->setSecurityMode(mode == 1);
                    resultData[0] = ReturnCodes::Success;
                    resultData[1] = serviceId;
                    resultLength = 2;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            },
            // State Callback of PID_SECURITY_MODE
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
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
                    resultData[2] = obj->isSecurityModeEnabled() ? 1 : 0;
                    resultLength = 3;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            }),
        new DataProperty( PID_P2P_KEY_TABLE, true, PDT_GENERIC_20, 1, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_GRP_KEY_TABLE, true, PDT_GENERIC_18, 50, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE, true, PDT_GENERIC_08, 32, ReadLv3 | WriteLv0 ), // written by ETS
        new FunctionProperty<SecurityInterfaceObject>(this, PID_SECURITY_FAILURES_LOG,
            // Command Callback of PID_SECURITY_FAILURES_LOG
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
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
                    obj->clearFailureLog();
                    resultData[0] = ReturnCodes::Success;
                    resultData[1] = id;
                    resultLength = 2;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            },
            // State Callback of PID_SECURITY_FAILURES_LOG
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
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
                    obj->getFailureCounters(&resultData[3]); // Put 8 bytes in the buffer
                    resultLength = 3 + 8;
                    return;
                }
                // query latest failure by index
                else if(id == 1)
                {
                    uint8_t maxBufferSize = resultLength; // Remember the maximum buffer size of the buffer that is provided to us
                    uint8_t index = info;
                    uint8_t numBytes = obj->getFromFailureLogByIndex(index, &resultData[2], maxBufferSize);
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
        new DataProperty( PID_TOOL_KEY, true, PDT_GENERIC_16, 1, ReadLv3 | WriteLv0, (uint8_t*) _fdsk ), // default is FDSK // ETS changes this property during programming from FDSK to some random key!
        new DataProperty( PID_SECURITY_REPORT, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, _secReport ), // Not implemented
        new DataProperty( PID_SECURITY_REPORT_CONTROL, true, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, _secReportCtrl ), // Not implemented
        new DataProperty( PID_SEQUENCE_NUMBER_SENDING, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0 ), // Updated by our device accordingly
        new DataProperty( PID_ZONE_KEY_TABLE, true, PDT_GENERIC_19, 1, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_GO_SECURITY_FLAGS, true, PDT_GENERIC_01, 256, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_ROLE_TABLE, true, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_ERROR_CODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0, (uint8_t)E_NO_FAULT),
        new DataProperty( PID_TOOL_SEQUENCE_NUMBER_SENDING, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0 ) // Updated by our device accordingly (non-standardized!)
    };
    initializeProperties(sizeof(properties), properties);
}

uint8_t* SecurityInterfaceObject::save(uint8_t* buffer)
{
    buffer = pushByte(_state, buffer);
    buffer = pushByte(_securityModeEnabled, buffer);

    return InterfaceObject::save(buffer);
}

const uint8_t* SecurityInterfaceObject::restore(const uint8_t* buffer)
{
    uint8_t state = 0;
    buffer = popByte(state, buffer);
    _state = (LoadState)state;

    uint8_t securityModeEnabled = 0;
    buffer = popByte(securityModeEnabled, buffer);
    _securityModeEnabled = securityModeEnabled;

    return InterfaceObject::restore(buffer);
}

uint16_t SecurityInterfaceObject::saveSize()
{
    return 2 + InterfaceObject::saveSize();
}

void SecurityInterfaceObject::setSecurityMode(bool enabled)
{
    print("Security mode set to: ");
    println(enabled ? "enabled" : "disabled");
    _securityModeEnabled = enabled;
}

bool SecurityInterfaceObject::isSecurityModeEnabled()
{
    return _securityModeEnabled;
}

void SecurityInterfaceObject::clearFailureLog()
{
    println("clearFailureLog()");
}

void SecurityInterfaceObject::getFailureCounters(uint8_t* data)
{
    memset(data, 0, 8);
    println("getFailureCounters()");
}

uint8_t SecurityInterfaceObject::getFromFailureLogByIndex(uint8_t index, uint8_t* data, uint8_t maxDataLen)
{
    print("getFromFailureLogByIndex(): Index: ");
    println(index);
    return 0;
}

bool SecurityInterfaceObject::isLoaded()
{
    return _state == LS_LOADED;
}

LoadState SecurityInterfaceObject::loadState()
{
    return _state;
}

void SecurityInterfaceObject::loadEvent(const uint8_t* data)
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

void SecurityInterfaceObject::loadEventUnloaded(const uint8_t* data)
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

void SecurityInterfaceObject::loadEventLoading(const uint8_t* data)
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

void SecurityInterfaceObject::loadEventLoaded(const uint8_t* data)
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

void SecurityInterfaceObject::loadEventError(const uint8_t* data)
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

void SecurityInterfaceObject::loadState(LoadState newState)
{
    if (newState == _state)
        return;
    //beforeStateChange(newState);
    _state = newState;
}

void SecurityInterfaceObject::errorCode(ErrorCode errorCode)
{
    uint8_t data = errorCode;
    Property* prop = property(PID_ERROR_CODE);
    prop->write(data);
}

void SecurityInterfaceObject::masterReset(EraseCode eraseCode, uint8_t channel)
{
    if (eraseCode == FactoryReset)
    {
        // TODO handle different erase codes
        println("Factory reset of security interface object requested.");
        setSecurityMode(false);
        property(PID_TOOL_KEY)->write(1, 1, _fdsk);
    }
}

const uint8_t* SecurityInterfaceObject::toolKey()
{
    // There is only one tool key
    const uint8_t* toolKey = propertyData(PID_TOOL_KEY);
    return toolKey;
}

const uint8_t* SecurityInterfaceObject::p2pKey(uint16_t addressIndex)
{
    if (!isLoaded())
        return nullptr;

    // Get number of entries for this property
    uint16_t numElements = getNumberOfElements(PID_P2P_KEY_TABLE);

    if (numElements > 0)
    {
        uint8_t elementSize = propertySize(PID_P2P_KEY_TABLE);

        // Search for address index
        uint8_t entry[elementSize]; // 2 bytes index + keysize (16 bytes) + 2 bytes(roles) = 20 bytes
        for (int i = 1; i <= numElements; i++)
        {
            property(PID_P2P_KEY_TABLE)->read(i, 1, entry);
            uint16_t index = (entry[0] << 8) | entry[1];
            if (index > addressIndex)
            {
                return nullptr;
            }
            if (index == addressIndex)
            {
                return propertyData(PID_P2P_KEY_TABLE, i) + sizeof(index);
            }
        }
    }

    return nullptr;
}

const uint8_t* SecurityInterfaceObject::groupKey(uint16_t addressIndex)
{
    if (!isLoaded())
        return nullptr;

    // Get number of entries for this property
    uint16_t numElements = getNumberOfElements(PID_GRP_KEY_TABLE);

    if (numElements > 0)
    {
        uint8_t elementSize = propertySize(PID_GRP_KEY_TABLE);

        // Search for address index
        uint8_t entry[elementSize]; // 2 bytes index + keysize (16 bytes) = 18 bytes
        for (int i = 1; i <= numElements; i++)
        {
            property(PID_GRP_KEY_TABLE)->read(i, 1, entry);
            uint16_t index = ((entry[0] << 8) | entry[1]);
            if (index > addressIndex)
            {
                return nullptr;
            }
            if (index == addressIndex)
            {
                return propertyData(PID_GRP_KEY_TABLE, i) + sizeof(index);
            }
        }
    }

    return nullptr;
}

uint16_t SecurityInterfaceObject::indAddressIndex(uint16_t indAddr)
{
    // Get number of entries for this property
    uint16_t numElements = getNumberOfElements(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE);

    if (numElements > 0)
    {
        uint8_t elementSize = propertySize(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE);

        // Search for individual address
        uint8_t entry[elementSize]; // 2 bytes address + 6 bytes seqno = 8 bytes
        for (int i = 1; i <= numElements; i++)
        {
            property(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE)->read(i, 1, entry);
            uint16_t addr = (entry[0] << 8) | entry[1];
            if (addr == indAddr)
            {
                return i;
            }
        }
    }

    // Not found
    return 0;
}

void SecurityInterfaceObject::setSequenceNumber(bool toolAccess, uint64_t seqNum)
{
    uint8_t seqBytes[6] = {0x00};
    sixBytesFromUInt64(seqNum, seqBytes);

    if (toolAccess)
    {
        property(PID_TOOL_SEQUENCE_NUMBER_SENDING)->write(1, 1, seqBytes);
    }
    else
    {
        property(PID_SEQUENCE_NUMBER_SENDING)->write(1, 1, seqBytes);
    }
}

uint16_t SecurityInterfaceObject::getNumberOfElements(PropertyID propId)
{
    // Get number of entries for this property
    uint16_t numElements = 0;

    uint8_t data[sizeof(uint16_t)]; // is sizeof(_currentElements) which is uint16_t
    uint8_t count = property(propId)->read(0, 1, data);

    if (count > 0)
    {
        popWord(numElements, data);
    }

    return numElements;
}

uint64_t SecurityInterfaceObject::getLastValidSequenceNumber(uint16_t deviceAddr)
{

    // Get number of entries for this property
    uint16_t numElements = getNumberOfElements(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE);

    if (numElements > 0)
    {
        uint8_t elementSize = propertySize(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE);

        // Search for individual address
        uint8_t entry[elementSize]; // 2 bytes address + 6 bytes seqno = 8 bytes
        for (int i = 1; i <= numElements; i++)
        {
            property(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE)->read(i, 1, entry);
            uint16_t addr = (entry[0] << 8) | entry[1];
            if (addr == deviceAddr)
            {
                return sixBytesToUInt64(&entry[2]);
            }
        }
    }
    return 0;
}

void SecurityInterfaceObject::setLastValidSequenceNumber(uint16_t deviceAddr, uint64_t seqNum)
{
    // Get number of entries for this property
    uint16_t numElements = getNumberOfElements(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE);

    if (numElements > 0)
    {
        uint8_t elementSize = propertySize(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE);

        // Search for individual address
        uint8_t entry[elementSize]; // 2 bytes address + 6 bytes seqno = 8 bytes
        for (int i = 1; i <= numElements; i++)
        {
            property(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE)->read(i, 1, entry);
            uint16_t addr = (entry[0] << 8) | entry[1];
            if (addr == deviceAddr)
            {
                sixBytesFromUInt64(seqNum, &entry[2]);
                property(PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE)->write(i, 1, entry);
            }
        }
    }
}

DataSecurity SecurityInterfaceObject::getGroupObjectSecurity(uint16_t index)
{
    // security table uses same index as group object table

    uint8_t data[propertySize(PID_GO_SECURITY_FLAGS)];

    uint8_t count = property(PID_GO_SECURITY_FLAGS)->read(index, 1, data);

    if (count > 0)
    {
        // write access flags, approved spec. AN158, p.97
        bool conf = (data[0] & 2) == 2;
        bool auth = (data[0] & 1) == 1;
        return conf ? DataSecurity::AuthConf : auth ? DataSecurity::Auth : DataSecurity::None;
    }

    return DataSecurity::None;
}

#endif

