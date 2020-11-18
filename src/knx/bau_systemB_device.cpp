#include "bau_systemB_device.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>

BauSystemBDevice::BauSystemBDevice(Platform& platform) :
    BauSystemB(platform),
    _addrTable(_memory),
    _assocTable(_memory), _groupObjTable(_memory),
#ifdef USE_DATASECURE
    _appLayer(_deviceObj, _secIfObj, *this),
#else
    _appLayer(*this),
#endif
    _transLayer(_appLayer), _netLayer(_deviceObj, _transLayer)
{
    _appLayer.transportLayer(_transLayer);
    _appLayer.associationTableObject(_assocTable);
#ifdef USE_DATASECURE
    _appLayer.groupAddressTable(_addrTable);
#endif
    _transLayer.networkLayer(_netLayer);
    _transLayer.groupAddressTable(_addrTable);

    _memory.addSaveRestore(&_deviceObj);
    _memory.addSaveRestore(&_addrTable);
    _memory.addSaveRestore(&_assocTable);
    _memory.addSaveRestore(&_groupObjTable);
#ifdef USE_DATASECURE
    _memory.addSaveRestore(&_secIfObj);
#endif
}

ApplicationLayer& BauSystemBDevice::applicationLayer()
{
    return _appLayer;
}

GroupObjectTableObject& BauSystemBDevice::groupObjectTable()
{
    return _groupObjTable;
}

void BauSystemBDevice::loop()
{
    _transLayer.loop();
    sendNextGroupTelegram();
    nextRestartState();
#ifdef USE_DATASECURE
    _appLayer.loop();
#endif
}

void BauSystemBDevice::sendNextGroupTelegram()
{
    if(!configured())
        return;
    
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

        SecurityControl goSecurity;
        goSecurity.toolAccess = false; // Secured group communication never uses the toolkey. ETS knows all keys, also the group keys.

#ifdef USE_DATASECURE
        // Get security flags from Security Interface Object for this group object
        goSecurity.dataSecurity = _secIfObj.getGroupObjectSecurity(asap);
#else
        goSecurity.dataSecurity = DataSecurity::None;
#endif

        if (flag == WriteRequest && go.transmitEnable())
        {
            uint8_t* data = go.valueRef();
            _appLayer.groupValueWriteRequest(AckRequested, asap, go.priority(), NetworkLayerParameter, goSecurity, data,
                go.sizeInTelegram());
        }
        else if (flag == ReadRequest)
        {
            _appLayer.groupValueReadRequest(AckRequested, asap, go.priority(), NetworkLayerParameter, goSecurity);
        }

        go.commFlag(Transmitting);

        startIdx = asap + 1;
        return;
    }

    startIdx = 1;
}

void BauSystemBDevice::updateGroupObject(GroupObject & go, uint8_t * data, uint8_t length)
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

bool BauSystemBDevice::configured()
{
    // _configured is set to true initially, if the device was configured with ETS it will be set to true after restart
    
    if (!_configured)
        return false;
    
    _configured = _groupObjTable.loadState() == LS_LOADED
        && _addrTable.loadState() == LS_LOADED
        && _assocTable.loadState() == LS_LOADED
        && _appProgram.loadState() == LS_LOADED;

#ifdef USE_DATASECURE
    _configured &= _secIfObj.loadState() == LS_LOADED;
#endif

    return _configured;
}

void BauSystemBDevice::doMasterReset(EraseCode eraseCode, uint8_t channel)
{
    BauSystemB::doMasterReset(eraseCode, channel);

    _addrTable.masterReset(eraseCode, channel);
    _assocTable.masterReset(eraseCode, channel);
    _groupObjTable.masterReset(eraseCode, channel);
#ifdef USE_DATASECURE
    _secIfObj.masterReset(eraseCode, channel);
#endif
}

void BauSystemBDevice::groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t * data, uint8_t dataLength, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void BauSystemBDevice::groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, bool status)
{
    GroupObject& go = _groupObjTable.get(asap);
    if (status)
        go.commFlag(Ok);
    else
        go.commFlag(Error);
}

void BauSystemBDevice::groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl)
{
#ifdef USE_DATASECURE
    DataSecurity requiredGoSecurity;

    // Get security flags from Security Interface Object for this group object
    requiredGoSecurity = _secIfObj.getGroupObjectSecurity(asap);

    if (secCtrl.dataSecurity != requiredGoSecurity)
    {
        println("GroupValueRead: access denied due to wrong security flags");
        return;
    }
#endif

    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.readEnable())
        return;
    
    uint8_t* data = go.valueRef();
    _appLayer.groupValueReadResponse(AckRequested, asap, priority, hopType, secCtrl, data, go.sizeInTelegram());
}

void BauSystemBDevice::groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* data,
    uint8_t dataLength)
{
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.responseUpdateEnable())
        return;

    updateGroupObject(go, data, dataLength);
}

void BauSystemBDevice::groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t * data, uint8_t dataLength)
{
#ifdef USE_DATASECURE
    DataSecurity requiredGoSecurity;

    // Get security flags from Security Interface Object for this group object
    requiredGoSecurity = _secIfObj.getGroupObjectSecurity(asap);

    if (secCtrl.dataSecurity != requiredGoSecurity)
    {
        println("GroupValueWrite: access denied due to wrong security flags");
        return;
    }
#endif
    GroupObject& go = _groupObjTable.get(asap);

    if (!go.communicationEnable() || !go.writeEnable())
        return;

    updateGroupObject(go, data, dataLength);
}
