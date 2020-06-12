#include "config.h"
#ifdef USE_DATASECURE

#include <cstring>
#include "security_interface_object.h"
#include "bits.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

// Our FDSK
uint8_t SecurityInterfaceObject::_fdsk[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

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
                obj->_state = (LoadState) data[0];
                return 1;
            }),
        new FunctionProperty<SecurityInterfaceObject>(this, PID_SECURITY_MODE, ReadLv3 | WriteLv0,
            // Command Callback of PID_SECURITY_MODE
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                // TODO
            },
            // State Callback of PID_SECURITY_MODE
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                // TODO
            }),
        new DataProperty( PID_P2P_KEY_TABLE, true, PDT_GENERIC_20, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_GRP_KEY_TABLE, true, PDT_GENERIC_18, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_SECURITY_INDIVIDUAL_ADDRESS_TABLE, true, PDT_GENERIC_08, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new FunctionProperty<SecurityInterfaceObject>(this, PID_SECURITY_FAILURES_LOG, ReadLv3 | WriteLv0,
            // Command Callback of PID_SECURITY_FAILURES_LOG
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                // TODO
            },
            // State Callback of PID_SECURITY_FAILURES_LOG
            [](SecurityInterfaceObject* obj, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) -> void {
                // TODO
            }),
        new DataProperty( PID_TOOL_KEY, true, PDT_GENERIC_16, 1, ReadLv3 | WriteLv0, (uint8_t*) _fdsk ), // default is FDSK
        new DataProperty( PID_SECURITY_REPORT, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_SECURITY_REPORT_CONTROL, true, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_SEQUENCE_NUMBER_SENDING, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_ZONE_KEY_TABLE, true, PDT_GENERIC_19, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_GO_SECURITY_FLAGS, true, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
        new DataProperty( PID_ROLE_TABLE, true, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint16_t)0 ), // TODO: value
    };
    initializeProperties(sizeof(properties), properties);
}

uint8_t* SecurityInterfaceObject::save(uint8_t* buffer)
{
    //buffer = pushWord(_ownAddress, buffer);
    return InterfaceObject::save(buffer);
}

const uint8_t* SecurityInterfaceObject::restore(const uint8_t* buffer)
{
    //buffer = popWord(_ownAddress, buffer);
    return InterfaceObject::restore(buffer);
}

uint16_t SecurityInterfaceObject::saveSize()
{
    //return 2 + InterfaceObject::saveSize();
    return InterfaceObject::saveSize();
}

bool SecurityInterfaceObject::isLoaded()
{
    return _state == LS_LOADED;
}

#endif

