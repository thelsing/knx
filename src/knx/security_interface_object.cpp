#include "config.h"
#ifdef USE_DATASECURE

#include <cstring>
#include "security_interface_object.h"
#include "secure_application_layer.h"
#include "bits.h"
#include "data_property.h"
#include "callback_property.h"
#include "function_property.h"

// Our FDSK. It is never changed from ETS. This is permanent default tool key restarted on every factory reset of the device.
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
                obj->_state = (LoadState) data[0];
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
                    obj->_secAppLayer->setSecurityMode(mode == 1);
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
                    resultData[2] = obj->_secAppLayer->isSecurityModeEnabled() ? 1 : 0;
                    resultLength = 3;
                    return;
                }
                resultData[0] = ReturnCodes::GenericError;
                resultLength = 1;
            }),
        new DataProperty( PID_P2P_KEY_TABLE, true, PDT_GENERIC_20, 32, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_GRP_KEY_TABLE, true, PDT_GENERIC_18, 32, ReadLv3 | WriteLv0 ), // written by ETS
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
                    obj->_secAppLayer->clearFailureLog();
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
                    obj->_secAppLayer->getFailureCounters(&resultData[3]); // Put 8 bytes in the buffer
                    resultLength = 3 + 8;
                    return;
                }
                // query latest failure by index
                else if(id == 1)
                {
                    uint8_t maxBufferSize = resultLength; // Remember the maximum buffer size of the buffer that is provided to us
                    uint8_t index = info;
                    uint8_t numBytes = obj->_secAppLayer->getFromFailureLogByIndex(index, &resultData[2], maxBufferSize);
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
        new DataProperty( PID_TOOL_KEY, true, PDT_GENERIC_16, 1, ReadLv3 | WriteLv0, (uint8_t*) _fdsk ), // default is FDSK // TODO: do not overwrite on every device startup!!!!!!!!! ETS changes this property during programming from FDSK to some random key!
        new DataProperty( PID_SECURITY_REPORT, true, PDT_BITSET8, 1, ReadLv3 | WriteLv0, _secReport ), // Not implemented
        new DataProperty( PID_SECURITY_REPORT_CONTROL, true, PDT_BINARY_INFORMATION, 1, ReadLv3 | WriteLv0, _secReportCtrl ), // Not implemented
        new DataProperty( PID_SEQUENCE_NUMBER_SENDING, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0 ), // Updated by our device accordingly
        new DataProperty( PID_ZONE_KEY_TABLE, true, PDT_GENERIC_19, 32, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_GO_SECURITY_FLAGS, true, PDT_GENERIC_01, 32, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_ROLE_TABLE, true, PDT_GENERIC_01, 32, ReadLv3 | WriteLv0 ), // written by ETS
        new DataProperty( PID_TOOL_SEQUENCE_NUMBER_SENDING, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv0 ) // Updated by our device accordingly (non-standardized!)
    };
    initializeProperties(sizeof(properties), properties);
}

void SecurityInterfaceObject::secureApplicationLayer(SecureApplicationLayer& secAppLayer)
{
    _secAppLayer = &secAppLayer;
}

uint8_t* SecurityInterfaceObject::save(uint8_t* buffer)
{
    return InterfaceObject::save(buffer);
}

const uint8_t* SecurityInterfaceObject::restore(const uint8_t* buffer)
{
    return InterfaceObject::restore(buffer);
}

uint16_t SecurityInterfaceObject::saveSize()
{
    return InterfaceObject::saveSize();
}

bool SecurityInterfaceObject::isLoaded()
{
    return _state == LS_LOADED;
}

void SecurityInterfaceObject::factoryReset()
{
    _secAppLayer->setSecurityMode(false);
    property(PID_TOOL_KEY)->write(1, 1, _fdsk);
}

#endif

