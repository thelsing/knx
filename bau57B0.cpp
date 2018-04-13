#include "bau57B0.h"
#include <string.h>
#include <stdio.h>


using namespace std;

Bau57B0::Bau57B0(Platform& platform): _memoryReference((uint8_t*)&_deviceObj), _memory(platform), _addrTable(_memoryReference), 
    _assocTable(_memoryReference), _groupObjTable(_memoryReference), _appProgram(_memoryReference),
    _ipParameters(_deviceObj, platform), _platform(platform), _appLayer(_assocTable, *this),
    _transLayer(_appLayer, _addrTable, _platform), _netLayer(_transLayer), 
    _dlLayer(_deviceObj, _addrTable, _ipParameters, _netLayer, _platform)
{
    _appLayer.transportLayer(_transLayer);
    _transLayer.networkLayer(_netLayer);
    _netLayer.dataLinkLayer(_dlLayer);
    _memory.addSaveRestore(&_deviceObj);
    _memory.addSaveRestore(&_ipParameters);
    _memory.addSaveRestore(&_appProgram);
    _memory.addSaveRestore(&_addrTable);
    _memory.addSaveRestore(&_assocTable);
    _memory.addSaveRestore(&_groupObjTable);
}

void Bau57B0::loop()
{
    _dlLayer.loop();
    _transLayer.loop();
    sendNextGroupTelegram();
}

void Bau57B0::sendNextGroupTelegram()
{
    static uint16_t startIdx = 1;

    GroupObjectTableObject& table = _groupObjTable;
    uint16_t objCount = table.entryCount();

    for (uint16_t asap = startIdx; asap < objCount; asap++)
    {
        GroupObject& go = table.get(asap);

        ComFlag flag = go.commFlag();
        if (flag != ReadRequest && flag != WriteRequest)
            continue;

        if(!go.communicationEnable() || ! go.transmitEnable())
            continue;

        if (flag == WriteRequest)
        {
            uint8_t* data = go.valueRef();
            _appLayer.groupValueWriteRequest(AckRequested, asap, go.priority(), NetworkLayerParameter, data, 
                go.sizeInTelegram());
        }
        else
        {
            _appLayer.groupValueReadRequest(AckRequested, asap, go.priority(), NetworkLayerParameter);
        }

        go.commFlag(Transmitting);

        startIdx = asap + 1;
        return;
    }

    startIdx = 1;
}

void Bau57B0::updateGroupObject(GroupObject & go, uint8_t * data, uint8_t length)
{
    uint8_t* goData = go.valueRef();
    if (length != go.valueSize())
    {
        go.commFlag(Error);
        return;
    }

    memcpy(goData, data, length);

    go.commFlag(cfUpdate);
    if (go.updateHandler)
        go.updateHandler(go);
}

void Bau57B0::readMemory()
{
    _memory.readMemory();
}

DeviceObject& Bau57B0::deviceObject()
{
    return _deviceObj;
}

GroupObjectTableObject& Bau57B0::groupObjectTable()
{
    return _groupObjTable;
}

ApplicationProgramObject& Bau57B0::parameters()
{
    return _appProgram;
}

bool Bau57B0::configured()
{
    return _groupObjTable.loadState() == LS_LOADED
        && _addrTable.loadState() == LS_LOADED
        && _assocTable.loadState() == LS_LOADED
        && _appProgram.loadState() == LS_LOADED;
}

bool Bau57B0::enabled()
{
    return _dlLayer.enabled();
}

void Bau57B0::enabled(bool value)
{
    _dlLayer.enabled(value);
}

void Bau57B0::memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, 
    uint16_t memoryAddress, uint8_t * data)
{
    memcpy(_memoryReference + memoryAddress, data, number);
    _memory.memoryModified();

    if (_deviceObj.verifyMode())
        memoryReadIndication(priority, hopType, asap, number, memoryAddress);
}

void Bau57B0::memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, 
    uint16_t memoryAddress)
{
    _appLayer.memoryReadResponse(AckRequested, priority, hopType, asap, number, memoryAddress,
        _memoryReference + memoryAddress);
}

void Bau57B0::deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t descriptorType)
{
    if (descriptorType != 0)
        descriptorType = 0x3f;
    
    uint8_t descriptor[] = { 0x57, 0xb0 };

    _appLayer.deviceDescriptorReadResponse(AckRequested, priority, hopType, asap, descriptorType, descriptor);
}

void Bau57B0::restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap)
{
    // for platforms that don't really restart
    _deviceObj.progMode(false);

    // Flush the EEPROM before resetting
    _memory.writeMemory();
    _platform.restart();
}

void Bau57B0::authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, uint32_t key)
{
    _appLayer.authorizeResponse(AckRequested, priority, hopType, asap, 0);
}

void Bau57B0::userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress)
{
    _appLayer.userMemoryReadResponse(AckRequested, priority, hopType, asap, number, memoryAddress,
        _memoryReference + memoryAddress);
}

void Bau57B0::userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
    memcpy(_memoryReference + memoryAddress, data, number);
    _memory.memoryModified();

    if (_deviceObj.verifyMode())
        userMemoryReadIndication(priority, hopType, asap, number, memoryAddress);
}

void Bau57B0::propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex, 
    uint8_t propertyId, uint8_t propertyIndex)
{
    bool writeEnable = false;
    uint8_t type = 0;
    uint16_t numberOfElements = 0;
    uint8_t access = 0;
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if (obj)
        obj->readPropertyDescription(propertyId, propertyIndex, writeEnable, type, numberOfElements, access);

    _appLayer.propertyDescriptionReadResponse(AckRequested, priority, hopType, asap, objectIndex, propertyId, propertyIndex,
        writeEnable, type, numberOfElements, access);
}

void Bau57B0::propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex, 
    uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    InterfaceObject* obj = getInterfaceObject(objectIndex);
    if(obj)
        obj->writeProperty((PropertyID)propertyId, startIndex, data, numberOfElements);
    propertyValueReadIndication(priority, hopType, asap, objectIndex, propertyId, numberOfElements, startIndex);
}

void Bau57B0::propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
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

void Bau57B0::individualAddressReadIndication(HopCountType hopType)
{
    if (_deviceObj.progMode())
        _appLayer.individualAddressReadResponse(AckRequested, hopType);
}

void Bau57B0::individualAddressWriteIndication(HopCountType hopType, uint16_t newaddress)
{
    if (_deviceObj.progMode())
        _deviceObj.induvidualAddress(newaddress);
}

void Bau57B0::groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void Bau57B0::groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void Bau57B0::groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType)
{
    GroupObject& go = _groupObjTable.get(asap);
    uint8_t* data = go.valueRef();
    _appLayer.groupValueReadResponse(AckRequested, asap, priority, hopType, data, go.sizeInTelegram());
}

void Bau57B0::groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, 
    uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);
    
    if (!go.communicationEnable() || !go.responseUpdateEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

void Bau57B0::groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.writeEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

InterfaceObject* Bau57B0::getInterfaceObject(uint8_t idx)
{
    switch (idx)
    {
    case 0:
        return &_deviceObj;
    case 1:
        return &_addrTable;
    case 2:
        return &_assocTable;
    case 3:
        return &_groupObjTable;
    case 4:
        return &_appProgram;
    case 5: // would be app_program 2
        return nullptr;
    case 6:
        return &_ipParameters;
    default:
        return nullptr;
    }
}