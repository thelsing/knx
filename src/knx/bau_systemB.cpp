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

BauSystemB::BauSystemB(Platform& platform): _memory(platform, _deviceObj), _addrTable(_memory),
    _assocTable(_memory), _groupObjTable(_memory), _appProgram(_memory),
    _platform(platform), _appLayer(_assocTable, *this),
    _transLayer(_appLayer, _addrTable), _netLayer(_transLayer)
{
    _appLayer.transportLayer(_transLayer);
    _transLayer.networkLayer(_netLayer);
    _memory.addSaveRestore(&_deviceObj);
    _memory.addSaveRestore(&_appProgram);
    _memory.addSaveRestore(&_addrTable);
    _memory.addSaveRestore(&_assocTable);
    _memory.addSaveRestore(&_groupObjTable);
}

void BauSystemB::loop()
{
    dataLinkLayer().loop();
    _transLayer.loop();
    sendNextGroupTelegram();
    nextRestartState();
}

bool BauSystemB::enabled()
{
    return dataLinkLayer().enabled();
}

void BauSystemB::enabled(bool value)
{
    dataLinkLayer().enabled(value);
}

void BauSystemB::sendNextGroupTelegram()
{
    static uint16_t startIdx = 1;

    GroupObjectTableObject& table = _groupObjTable;
    uint16_t objCount = table.entryCount();

    for (uint16_t asap = startIdx; asap <= objCount; asap++)
    {
        GroupObject& go = table.get(asap);

        ComFlag flag = go.commFlag();
        if (flag != ReadRequest && flag != WriteRequest)
            continue;

        if (!go.communicationEnable())
            continue;

        if (flag == WriteRequest && go.transmitEnable())
        {
            uint8_t* data = go.valueRef();
            _appLayer.groupValueWriteRequest(AckRequested, asap, go.priority(), NetworkLayerParameter, data,
                go.sizeInTelegram());
        }
        else if (flag == ReadRequest)
        {
            _appLayer.groupValueReadRequest(AckRequested, asap, go.priority(), NetworkLayerParameter);
        }

        go.commFlag(Transmitting);

        startIdx = asap + 1;
        return;
    }

    startIdx = 1;
}

void BauSystemB::updateGroupObject(GroupObject & go, uint8_t * data, uint8_t length)
{
    uint8_t* goData = go.valueRef();
    if (length != go.valueSize())
    {
        go.commFlag(Error);
        return;
    }

    memcpy(goData, data, length);

    go.commFlag(Updated);
    GroupObjectUpdatedHandler handler = go.callback();
    if (handler)
        handler(go);
}

void BauSystemB::readMemory()
{
    _memory.readMemory();
}

void BauSystemB::writeMemory()
{
    _memory.writeMemory();
}

DeviceObject& BauSystemB::deviceObject()
{
    return _deviceObj;
}

GroupObjectTableObject& BauSystemB::groupObjectTable()
{
    return _groupObjTable;
}

ApplicationProgramObject& BauSystemB::parameters()
{
    return _appProgram;
}

bool BauSystemB::configured()
{
    // _configured is set to true initially, if the device was configured with ETS it will be set to true after restart
    
    if (!_configured)
        return false;
    
    _configured = _groupObjTable.loadState() == LS_LOADED
        && _addrTable.loadState() == LS_LOADED
        && _assocTable.loadState() == LS_LOADED
        && _appProgram.loadState() == LS_LOADED;
    
    return _configured;
}

void BauSystemB::deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t descriptorType)
{
    if (descriptorType != 0)
        descriptorType = 0x3f;

    _appLayer.deviceDescriptorReadResponse(AckRequested, priority, hopType, asap, descriptorType, descriptor());
}

void BauSystemB::memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress, uint8_t * data)
{
    _memory.writeMemory(memoryAddress, number, data);

    if (_deviceObj.verifyMode())
        memoryReadIndication(priority, hopType, asap, number, memoryAddress);
}

void BauSystemB::memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress)
{
    _appLayer.memoryReadResponse(AckRequested, priority, hopType, asap, number, memoryAddress,
        _memory.toAbsolute(memoryAddress));
}

void BauSystemB::restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap)
{
    // Flush the EEPROM before resetting
    _memory.writeMemory();
    _platform.restart();
}

void BauSystemB::authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, uint32_t key)
{
    _appLayer.authorizeResponse(AckRequested, priority, hopType, asap, 0);
}

void BauSystemB::userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress)
{
    _appLayer.userMemoryReadResponse(AckRequested, priority, hopType, asap, number, memoryAddress,
        _memory.toAbsolute(memoryAddress));
}

void BauSystemB::userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
    _memory.writeMemory(memoryAddress, number, data);

    if (_deviceObj.verifyMode())
        userMemoryReadIndication(priority, hopType, asap, number, memoryAddress);
}

void BauSystemB::propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
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

    _appLayer.propertyDescriptionReadResponse(AckRequested, priority, hopType, asap, objectIndex, pid, propertyIndex,
        writeEnable, type, numberOfElements, access);
}

void BauSystemB::propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    uint32_t elementCount = numberOfElements;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if(obj)
        obj->writeProperty((PropertyID)propertyId, startIndex, data, elementCount);
    propertyValueReadIndication(priority, hopType, asap, objectIndex, propertyId, elementCount, startIndex);
}

void BauSystemB::propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
    uint8_t size = 0;
    uint32_t elementCount = numberOfElements;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if (obj)
    {
        uint8_t elementSize = obj->propertySize((PropertyID)propertyId);
        size = elementSize * numberOfElements;
    }
    else
        elementCount = 0;

    uint8_t data[size];
    if(obj)
        obj->readProperty((PropertyID)propertyId, startIndex, elementCount, data);
    _appLayer.propertyValueReadResponse(AckRequested, priority, hopType, asap, objectIndex, propertyId, elementCount,
        startIndex, data, size);
}

void BauSystemB::individualAddressReadIndication(HopCountType hopType)
{
    if (_deviceObj.progMode())
        _appLayer.individualAddressReadResponse(AckRequested, hopType);
}

void BauSystemB::individualAddressWriteIndication(HopCountType hopType, uint16_t newaddress)
{
    if (_deviceObj.progMode())
        _deviceObj.induvidualAddress(newaddress);
}

void BauSystemB::groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void BauSystemB::groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void BauSystemB::groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.readEnable())
        return;
    
    uint8_t* data = go.valueRef();
    _appLayer.groupValueReadResponse(AckRequested, asap, priority, hopType, data, go.sizeInTelegram());
}

void BauSystemB::groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data,
    uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.responseUpdateEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

void BauSystemB::groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.writeEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

void BauSystemB::addSaveRestore(SaveRestore* obj)
{
    _memory.addSaveRestore(obj);
}

bool BauSystemB::restartRequest(uint16_t asap)
{
    if (_appLayer.isConnected())
        return false;
    _restartState = Connecting; // order important, has to be set BEFORE connectRequest
    _appLayer.connectRequest(asap, SystemPriority);
    _appLayer.deviceDescriptorReadRequest(AckRequested, SystemPriority, NetworkLayerParameter, asap, 0);
    return true;
}

void BauSystemB::connectConfirm(uint16_t tsap)
{
    if (_restartState == Connecting && tsap >= 0)
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
                _appLayer.restartRequest(AckRequested, SystemPriority, NetworkLayerParameter);
                _restartState = Restarted;
                _restartDelay = millis();
            }
            break;
        case Restarted:
            /* restart is finished, we send a discommect */
            if (millis() - _restartDelay > 30)
            {
                _appLayer.disconnectRequest(SystemPriority);
                _restartState = Idle;
            }
        default:
            break;
    }
}

void BauSystemB::systemNetworkParameterReadIndication(Priority priority, HopCountType hopType, uint16_t objectType,
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
                _appLayer.systemNetworkParameterReadResponse(priority, hopType, objectType, propertyId,
                                                             testInfo, testInfoLength, (uint8_t*) _deviceObj.knxSerialNumber(), 6);
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

void BauSystemB::propertyValueRead(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                   uint32_t &numberOfElements, uint16_t startIndex,
                                   uint8_t **data, uint32_t &length)
{
    uint32_t size = 0;
    uint32_t elementCount = numberOfElements;

    InterfaceObject* obj = getInterfaceObject(objectType, objectInstance);

    if (obj)
    {
        uint8_t elementSize = obj->propertySize((PropertyID)propertyId);
        size = elementSize * numberOfElements;
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
                                    uint32_t &numberOfElements, uint16_t startIndex,
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