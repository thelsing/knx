#include "bau_systemB.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

enum NmReadSerialNumberType
{
    NM_Read_SerialNumber_By_ProgrammingMode = 0x01,
    NM_Read_SerialNumber_By_ExFactoryState = 0x02,
    NM_Read_SerialNumber_By_PowerReset = 0x03,
    NM_Read_SerialNumber_By_ManufacturerSpecific = 0xFE,
};

static constexpr auto kFunctionPropertyResultBufferMaxSize = 0xFF;
static constexpr auto kRestartProcessTime = 3;

BauSystemB::BauSystemB(Platform& platform): _memory(platform, _deviceObj),
     _appProgram(_memory),
    _platform(platform)
{
    _memory.addSaveRestore(&_appProgram);
}

void BauSystemB::readMemory()
{
    _memory.readMemory();
}

void BauSystemB::writeMemory()
{
    _memory.writeMemory();
}

Platform& BauSystemB::platform()
{
    return _platform;
}

ApplicationProgramObject& BauSystemB::parameters()
{
    return _appProgram;
}

DeviceObject& BauSystemB::deviceObject()
{
    return _deviceObj;
}

uint8_t BauSystemB::checkmasterResetValidity(EraseCode eraseCode, uint8_t channel)
{
    static constexpr uint8_t successCode = 0x00; // Where does this come from? It is the code for "success".
    static constexpr uint8_t invalidEraseCode = 0x02; // Where does this come from? It is the error code for "unspported erase code".

    switch (eraseCode)
    {
        case EraseCode::ConfirmedRestart:
        {
            println("Confirmed restart requested.");
            return successCode;
        }
        case EraseCode::ResetAP:
        {
            // TODO: increase download counter except for confirmed restart (PID_DOWNLOAD_COUNTER)
            println("ResetAP requested. Not implemented yet.");
            return successCode;
        }
        case EraseCode::ResetIA:
        {
            // TODO: increase download counter except for confirmed restart (PID_DOWNLOAD_COUNTER)
            println("ResetIA requested. Not implemented yet.");
            return successCode;
        }
        case EraseCode::ResetLinks:
        {
            // TODO: increase download counter except for confirmed restart (PID_DOWNLOAD_COUNTER)
            println("ResetLinks requested. Not implemented yet.");
            return successCode;
        }
        case EraseCode::ResetParam:
        {
            // TODO: increase download counter except for confirmed restart (PID_DOWNLOAD_COUNTER)
            println("ResetParam requested. Not implemented yet.");
            return successCode;
        }
        case EraseCode::FactoryReset:
        {
            // TODO: increase download counter except for confirmed restart (PID_DOWNLOAD_COUNTER)
            println("Factory reset requested. type: with IA");
            return successCode;
        }
        case EraseCode::FactoryResetWithoutIA:
        {
            // TODO: increase download counter except for confirmed restart (PID_DOWNLOAD_COUNTER)
            println("Factory reset requested. type: without IA");
            return successCode;
        }
        default:
        {
            print("Unhandled erase code: ");
            println(eraseCode, HEX);
            return invalidEraseCode;
        }
    }
}

void BauSystemB::deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptorType)
{
    if (descriptorType != 0)
        descriptorType = 0x3f;
    
    uint8_t data[2];
    pushWord(_deviceObj.maskVersion(), data);
    applicationLayer().deviceDescriptorReadResponse(AckRequested, priority, hopType, asap, secCtrl, descriptorType, data);
}

void BauSystemB::memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
    uint16_t memoryAddress, uint8_t * data)
{
    _memory.writeMemory(memoryAddress, number, data);
    if (_deviceObj.verifyMode())
        memoryReadIndication(priority, hopType, asap, secCtrl, number, memoryAddress, data);
}

void BauSystemB::memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
    uint16_t memoryAddress, uint8_t * data)
{
    applicationLayer().memoryReadResponse(AckRequested, priority, hopType, asap, secCtrl, number, memoryAddress, data);
}

void BauSystemB::memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
    uint16_t memoryAddress)
{
    applicationLayer().memoryReadResponse(AckRequested, priority, hopType, asap, secCtrl, number, memoryAddress,
        _memory.toAbsolute(memoryAddress));
}

void BauSystemB::memoryExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t * data)
{
    _memory.writeMemory(memoryAddress, number, data);

    applicationLayer().memoryExtWriteResponse(AckRequested, priority, hopType, asap, secCtrl, ReturnCodes::Success, number, memoryAddress, _memory.toAbsolute(memoryAddress));
}

void BauSystemB::memoryExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress)
{
    applicationLayer().memoryExtReadResponse(AckRequested, priority, hopType, asap, secCtrl, ReturnCodes::Success, number, memoryAddress, _memory.toAbsolute(memoryAddress));
}

void BauSystemB::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
    _deviceObj.masterReset(eraseCode, channel);
    _appProgram.masterReset(eraseCode, channel);
}

void BauSystemB::restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, RestartType restartType, EraseCode eraseCode, uint8_t channel)
{
    if (restartType == RestartType::BasicRestart)
    {
        println("Basic restart requested");
        if (_beforeRestart != 0)
            _beforeRestart();
    }
    else if (restartType == RestartType::MasterReset)
    {
        uint8_t errorCode = checkmasterResetValidity(eraseCode, channel);
        // We send the restart response now before actually applying the reset values
        // Processing time is kRestartProcessTime (example 3 seconds) that we require for the applying the master reset with restart
        applicationLayer().restartResponse(AckRequested, priority, hopType, secCtrl, errorCode, (errorCode == 0) ? kRestartProcessTime : 0);
        doMasterReset(eraseCode, channel);
    }
    else
    {
        // Cannot happen as restartType is just one bit
        println("Unhandled restart type.");
        _platform.fatalError();
    }

    // Flush the EEPROM before resetting
    _memory.writeMemory();
    _platform.restart();
}

void BauSystemB::authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint32_t key)
{
    applicationLayer().authorizeResponse(AckRequested, priority, hopType, asap, secCtrl, 0);
}

void BauSystemB::userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress)
{
    applicationLayer().userMemoryReadResponse(AckRequested, priority, hopType, asap, secCtrl, number, memoryAddress,
        _memory.toAbsolute(memoryAddress));
}

void BauSystemB::userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
    _memory.writeMemory(memoryAddress, number, data);

    if (_deviceObj.verifyMode())
        userMemoryReadIndication(priority, hopType, asap, secCtrl, number, memoryAddress);
}

void BauSystemB::propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
    uint8_t propertyId, uint8_t propertyIndex)
{
    uint8_t pid = propertyId;
    bool writeEnable = false;
    uint8_t type = 0;
    uint16_t numberOfElements = 0;
    uint8_t access = 0;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if (obj)
        obj->readPropertyDescription(pid, propertyIndex, writeEnable, type, numberOfElements, access);

    applicationLayer().propertyDescriptionReadResponse(AckRequested, priority, hopType, asap, secCtrl, objectIndex, pid, propertyIndex,
        writeEnable, type, numberOfElements, access);
}

void BauSystemB::propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if(obj)
        obj->writeProperty((PropertyID)propertyId, startIndex, data, numberOfElements);
    propertyValueReadIndication(priority, hopType, asap, secCtrl, objectIndex, propertyId, numberOfElements, startIndex);
}

void BauSystemB::propertyValueExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool confirmed)
{
    uint8_t returnCode = ReturnCodes::Success;

    InterfaceObject* obj = getInterfaceObject(objectType, objectInstance);
    if(obj)
        obj->writeProperty((PropertyID)propertyId, startIndex, data, numberOfElements);
    else
        returnCode = ReturnCodes::AddressVoid;

    if (confirmed)
    {
        applicationLayer().propertyValueExtWriteConResponse(AckRequested, priority, hopType, asap, secCtrl, objectType, objectInstance, propertyId, numberOfElements, startIndex, returnCode);
    }
}

void BauSystemB::propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
    uint8_t size = 0;
    uint8_t elementCount = numberOfElements;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if (obj)
    {
        uint8_t elementSize = obj->propertySize((PropertyID)propertyId);
        if (startIndex > 0)
            size = elementSize * numberOfElements;
        else
            size = sizeof(uint16_t); // size of property array entry 0 which contains the current number of elements
    }
    else
        elementCount = 0;

    uint8_t data[size];
    if(obj)
        obj->readProperty((PropertyID)propertyId, startIndex, elementCount, data);
    
    if (elementCount == 0)
        size = 0;
    
    applicationLayer().propertyValueReadResponse(AckRequested, priority, hopType, asap, secCtrl, objectIndex, propertyId, elementCount,
                                        startIndex, data, size);
}

void BauSystemB::propertyValueExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
    uint8_t size = 0;
    uint8_t elementCount = numberOfElements;
    InterfaceObject* obj = getInterfaceObject(objectType, objectInstance);
    if (obj)
    {
        uint8_t elementSize = obj->propertySize((PropertyID)propertyId);
        if (startIndex > 0)
            size = elementSize * numberOfElements;
        else
            size = sizeof(uint16_t); // size of propert array entry 0 which is the size
    }
    else
        elementCount = 0;

    uint8_t data[size];
    if(obj)
        obj->readProperty((PropertyID)propertyId, startIndex, elementCount, data);

    if (elementCount == 0)
        size = 0;

    applicationLayer().propertyValueExtReadResponse(AckRequested, priority, hopType, asap, secCtrl, objectType, objectInstance, propertyId, elementCount,
                                           startIndex, data, size);
}

void BauSystemB::functionPropertyCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                   uint8_t propertyId, uint8_t* data, uint8_t length)
{
    uint8_t resultData[kFunctionPropertyResultBufferMaxSize];
    uint8_t resultLength = sizeof(resultData); // tell the callee the maximum size of the buffer

    bool handled = false;

    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if(obj)
    {
        if (obj->property((PropertyID)propertyId)->Type() == PDT_FUNCTION)
        {
            obj->command((PropertyID)propertyId, data, length, resultData, resultLength);
            handled = true;
        }
        else
        {
            if(_functionProperty != 0)
                if(_functionProperty(objectIndex, propertyId, length, data, resultData, resultLength))
                    handled = true;
        }
    } else {
        if(_functionProperty != 0)
            if(_functionProperty(objectIndex, propertyId, length, data, resultData, resultLength))
                handled = true;
    }

    //only return a value it was handled by a property or function
    if(handled)
        applicationLayer().functionPropertyStateResponse(AckRequested, priority, hopType, asap, secCtrl, objectIndex, propertyId, resultData, resultLength);
}

void BauSystemB::functionPropertyStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                 uint8_t propertyId, uint8_t* data, uint8_t length)
{
    uint8_t resultData[kFunctionPropertyResultBufferMaxSize];
    uint8_t resultLength = sizeof(resultData); // tell the callee the maximum size of the buffer

    bool handled = true;

    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if(obj)
    {
        if (obj->property((PropertyID)propertyId)->Type() == PDT_FUNCTION)
        {
            obj->state((PropertyID)propertyId, data, length, resultData, resultLength);
            handled = true;
        }
        else
        {
            if(_functionPropertyState != 0)
                if(_functionPropertyState(objectIndex, propertyId, length, data, resultData, resultLength))
                    handled = true;
        }
    } else {
        if(_functionPropertyState != 0)
            if(_functionPropertyState(objectIndex, propertyId, length, data, resultData, resultLength))
                handled = true;
    }

    //only return a value it was handled by a property or function
    if(handled)
        applicationLayer().functionPropertyStateResponse(AckRequested, priority, hopType, asap, secCtrl, objectIndex, propertyId, resultData, resultLength);
}

void BauSystemB::functionPropertyExtCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                      uint8_t propertyId, uint8_t* data, uint8_t length)
{
    uint8_t resultData[kFunctionPropertyResultBufferMaxSize];
    uint8_t resultLength = 1; // we always have to include the return code at least

    InterfaceObject* obj = getInterfaceObject(objectType, objectInstance);
    if(obj)
    {
        PropertyDataType propType = obj->property((PropertyID)propertyId)->Type();

        if (propType == PDT_FUNCTION)
        {
            // The first byte is reserved and 0 for PDT_FUNCTION
            uint8_t reservedByte = data[0];
            if (reservedByte != 0x00)
            {
                resultData[0] = ReturnCodes::DataVoid;
            }
            else
            {
                resultLength = sizeof(resultData); // tell the callee the maximum size of the buffer
                obj->command((PropertyID)propertyId, data, length, resultData, resultLength);
                // resultLength was modified by the callee
            }
        }
        else if (propType == PDT_CONTROL)
        {
            uint8_t count = 1;
            // write the event
            obj->writeProperty((PropertyID)propertyId, 1, data, count);
            if (count == 1)
            {
                // Read the current state (one byte only) for the response
                obj->readProperty((PropertyID)propertyId, 1, count, &resultData[1]);
                resultLength = count ? 2 : 1;
                resultData[0] = count ? ReturnCodes::Success : ReturnCodes::DataVoid;
            }
            else
            {
                resultData[0] = ReturnCodes::AddressVoid;
            }
        }
        else
        {
            resultData[0] = ReturnCodes::DataTypeConflict;
        }
    }
    else
    {
        resultData[0] = ReturnCodes::GenericError;
    }

    applicationLayer().functionPropertyExtStateResponse(AckRequested, priority, hopType, asap, secCtrl, objectType, objectInstance, propertyId, resultData, resultLength);
}

void BauSystemB::functionPropertyExtStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                    uint8_t propertyId, uint8_t* data, uint8_t length)
{
    uint8_t resultData[kFunctionPropertyResultBufferMaxSize];
    uint8_t resultLength = sizeof(resultData); // tell the callee the maximum size of the buffer

    InterfaceObject* obj = getInterfaceObject(objectType, objectInstance);
    if(obj)
    {
        PropertyDataType propType = obj->property((PropertyID)propertyId)->Type();

        if (propType == PDT_FUNCTION)
        {
            // The first byte is reserved and 0 for PDT_FUNCTION
            uint8_t reservedByte = data[0];
            if (reservedByte != 0x00)
            {
                resultData[0] = ReturnCodes::DataVoid;
            }
            else
            {
                resultLength = sizeof(resultData); // tell the callee the maximum size of the buffer
                obj->state((PropertyID)propertyId, data, length, resultData, resultLength);
                // resultLength was modified by the callee
            }
        }
        else if (propType == PDT_CONTROL)
        {
            uint8_t count = 1;
            // Read the current state (one byte only) for the response
            obj->readProperty((PropertyID)propertyId, 1, count, &resultData[1]);
            resultLength = count ? 2 : 1;
            resultData[0] = count ? ReturnCodes::Success : ReturnCodes::DataVoid;
        }
        else
        {
            resultData[0] = ReturnCodes::DataTypeConflict;
        }
    }
    else
    {
        resultData[0] = ReturnCodes::GenericError;
    }

    applicationLayer().functionPropertyExtStateResponse(AckRequested, priority, hopType, asap, secCtrl, objectType, objectInstance, propertyId, resultData, resultLength);
}

void BauSystemB::individualAddressReadIndication(HopCountType hopType, const SecurityControl &secCtrl)
{
    if (_deviceObj.progMode())
        applicationLayer().individualAddressReadResponse(AckRequested, hopType, secCtrl);
}

void BauSystemB::individualAddressWriteIndication(HopCountType hopType, const SecurityControl &secCtrl, uint16_t newaddress)
{
    if (_deviceObj.progMode())
        _deviceObj.individualAddress(newaddress);
}

void BauSystemB::individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t newIndividualAddress,
                                                          uint8_t* knxSerialNumber)
{
    // If the received serial number matches our serial number
    // then store the received new individual address in the device object
    if (!memcmp(knxSerialNumber, _deviceObj.propertyData(PID_SERIAL_NUMBER), 6))
        _deviceObj.individualAddress(newIndividualAddress);
}

void BauSystemB::individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* knxSerialNumber)
{
    // If the received serial number matches our serial number
    // then send a response with the serial number. The domain address is set to 0 for closed media.
    // An open medium BAU has to override this method and provide a proper domain address.
    if (!memcmp(knxSerialNumber, _deviceObj.propertyData(PID_SERIAL_NUMBER), 6))
    {
        uint8_t emptyDomainAddress[2] = {0x00};
        applicationLayer().IndividualAddressSerialNumberReadResponse(priority, hopType, secCtrl, emptyDomainAddress, knxSerialNumber);
    }
}

void BauSystemB::addSaveRestore(SaveRestore* obj)
{
    _memory.addSaveRestore(obj);
}

bool BauSystemB::restartRequest(uint16_t asap, const SecurityControl secCtrl)
{
    if (applicationLayer().isConnected())
        return false;
    _restartState = Connecting; // order important, has to be set BEFORE connectRequest
    _restartSecurity = secCtrl;
    applicationLayer().connectRequest(asap, SystemPriority);
    applicationLayer().deviceDescriptorReadRequest(AckRequested, SystemPriority, NetworkLayerParameter, asap, secCtrl, 0);
    return true;
}

void BauSystemB::connectConfirm(uint16_t tsap)
{
    if (_restartState == Connecting)
    {
        /* restart connection is confirmed, go to the next state */
        _restartState = Connected;
        _restartDelay = millis();
    }
    else
    {
        _restartState = Idle;
    }
}

void BauSystemB::nextRestartState()
{
    switch (_restartState)
    {
        case Idle:
            /* inactive state, do nothing */
            break;
        case Connecting:
            /* wait for connection, we do nothing here */
            break;
        case Connected:
            /* connection confirmed, we send restartRequest, but we wait a moment (sending ACK etc)... */
            if (millis() - _restartDelay > 30)
            {
                applicationLayer().restartRequest(AckRequested, SystemPriority, NetworkLayerParameter, _restartSecurity);
                _restartState = Restarted;
                _restartDelay = millis();
            }
            break;
        case Restarted:
            /* restart is finished, we send a disconnect */
            if (millis() - _restartDelay > 30)
            {
                applicationLayer().disconnectRequest(SystemPriority);
                _restartState = Idle;
            }
        default:
            break;
    }
}

void BauSystemB::systemNetworkParameterReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                      uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength)
{
    uint8_t operand;

    popByte(operand, testInfo + 1); // First byte (+ 0) contains only 4 reserved bits (0)

    // See KNX spec. 3.5.2 p.33 (Management Procedures: Procedures with A_SystemNetworkParameter_Read)
    switch((NmReadSerialNumberType)operand)
    {
        case NM_Read_SerialNumber_By_ProgrammingMode: // NM_Read_SerialNumber_By_ProgrammingMode
            // Only send a reply if programming mode is on
            if (_deviceObj.progMode() && (objectType == OT_DEVICE) && (propertyId == PID_SERIAL_NUMBER))
            {
                // Send reply. testResult data is KNX serial number
                applicationLayer().systemNetworkParameterReadResponse(priority, hopType, secCtrl, objectType, propertyId,
                                                             testInfo, testInfoLength, (uint8_t*)_deviceObj.propertyData(PID_SERIAL_NUMBER), 6);
            }
        break;

        case NM_Read_SerialNumber_By_ExFactoryState: // NM_Read_SerialNumber_By_ExFactoryState
        break;

        case NM_Read_SerialNumber_By_PowerReset: // NM_Read_SerialNumber_By_PowerReset
        break;

        case NM_Read_SerialNumber_By_ManufacturerSpecific: // Manufacturer specific use of A_SystemNetworkParameter_Read
        break;
    }
}

void BauSystemB::systemNetworkParameterReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                         uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength, bool status)
{
}

void BauSystemB::propertyValueRead(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                   uint8_t &numberOfElements, uint16_t startIndex,
                                   uint8_t **data, uint32_t &length)
{
    uint32_t size = 0;
    uint8_t elementCount = numberOfElements;

    InterfaceObject* obj = getInterfaceObject(objectType, objectInstance);

    if (obj)
    {
        uint8_t elementSize = obj->propertySize((PropertyID)propertyId);
        if (startIndex > 0)
            size = elementSize * numberOfElements;
        else
            size = sizeof(uint16_t); // size of property array entry 0 which contains the current number of elements
        *data = new uint8_t [size];
        obj->readProperty((PropertyID)propertyId, startIndex, elementCount, *data);
    }
    else
    {
        elementCount = 0;
        *data = nullptr;
    }

    numberOfElements = elementCount;
    length = size;
}

void BauSystemB::propertyValueWrite(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                    uint8_t &numberOfElements, uint16_t startIndex,
                                    uint8_t* data, uint32_t length)
{
    InterfaceObject* obj =  getInterfaceObject(objectType, objectInstance);
    if(obj)
        obj->writeProperty((PropertyID)propertyId, startIndex, data, numberOfElements);
    else 
        numberOfElements = 0;
}

Memory& BauSystemB::memory()
{
    return _memory;
}

void BauSystemB::versionCheckCallback(VersionCheckCallback func)
{
    _memory.versionCheckCallback(func);
}

VersionCheckCallback BauSystemB::versionCheckCallback()
{
    return _memory.versionCheckCallback();
}

void BauSystemB::beforeRestartCallback(BeforeRestartCallback func)
{
    _beforeRestart = func;
}

BeforeRestartCallback BauSystemB::beforeRestartCallback()
{
    return _beforeRestart;
}

void BauSystemB::functionPropertyCallback(FunctionPropertyCallback func)
{
    _functionProperty = func;
}

FunctionPropertyCallback BauSystemB::functionPropertyCallback()
{
    return _functionProperty;
}
void BauSystemB::functionPropertyStateCallback(FunctionPropertyCallback func)
{
    _functionPropertyState = func;
}

FunctionPropertyCallback BauSystemB::functionPropertyStateCallback()
{
    return _functionPropertyState;
}