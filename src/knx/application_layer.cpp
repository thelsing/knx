#include "application_layer.h"
#include "transport_layer.h"
#include "cemi_frame.h"
#include "association_table_object.h"
#include "apdu.h"
#include "bau.h"
#include "string.h"
#include "bits.h"
#include <stdio.h>

ApplicationLayer::ApplicationLayer(AssociationTableObject& assocTable, BusAccessUnit& bau):
    _assocTable(assocTable),  _bau(bau)
{
}

void ApplicationLayer::transportLayer(TransportLayer& layer)
{
    _transportLayer = &layer;
}

#pragma region TL Callbacks

void ApplicationLayer::dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    uint8_t len = apdu.length();
    uint8_t dataArray[len];
    uint8_t* data = dataArray;
    memcpy(data, apdu.data(), len);
    if (len == 1)
    {
        //less than six bit are encoded in first byte
        *data &= 0x3f;
    }
    else
    {
        data += 1;
        len -= 1;
    }

    uint16_t startIdx = 0;
    int32_t asap = _assocTable.nextAsap(tsap, startIdx);
    for (; asap != -1; asap = _assocTable.nextAsap(tsap, startIdx))
    {
        switch (apdu.type())
        {
            case GroupValueRead:
                _bau.groupValueReadIndication(asap, priority, hopType);
                break;
            case GroupValueResponse:
                _bau.groupValueReadAppLayerConfirm(asap, priority, hopType, data, len);
                break;
            case GroupValueWrite:
                _bau.groupValueWriteIndication(asap, priority, hopType, data, len);
            default:
                /* other apdutypes ar not valid here. If the appear do nothing */
                break;
        }
    }
}

void ApplicationLayer::dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority,  uint16_t tsap, APDU& apdu, bool status)
{
    switch (apdu.type())
    {
    case GroupValueRead:
        _bau.groupValueReadLocalConfirm(ack, _savedAsapReadRequest, priority, hopType, status);
        break;
    case GroupValueResponse:
        _bau.groupValueReadResponseConfirm(ack, _savedAsapResponse, priority, hopType, apdu.data(), apdu.length() - 1, status);
        break;
    case GroupValueWrite:
        _bau.groupValueWriteLocalConfirm(ack, _savedAsapWriteRequest, priority, hopType, apdu.data(), apdu.length() - 1, status);
        break;
    default:
        print("datagroup-confirm: unhandled APDU-Type: ");
        println(apdu.type());
    }
}

void ApplicationLayer::dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu, SystemBroadcast broadcastType)
{
    uint8_t* data = apdu.data();

    if (broadcastType == Broadcast)
    {
        // APCI on Broadcast
        switch (apdu.type())
        {
            case IndividualAddressWrite:
            {
                uint16_t newAddress;
                popWord(newAddress, data + 1);
                _bau.individualAddressWriteIndication(hopType, newAddress);
                break;
            }
            case IndividualAddressRead:
                _bau.individualAddressReadIndication(hopType);
                break;
            case IndividualAddressResponse:
                _bau.individualAddressReadAppLayerConfirm(hopType, apdu.frame().sourceAddress());
                break;
            case IndividualAddressSerialNumberRead:
            {
                uint8_t* knxSerialNumber = &data[1];
                _bau.individualAddressSerialNumberReadIndication(priority, hopType, knxSerialNumber);
                break;
            }
            case IndividualAddressSerialNumberResponse:
            {
                uint16_t domainAddress;
                popWord(domainAddress, data + 7);
                _bau.individualAddressSerialNumberReadAppLayerConfirm(hopType, data + 1, apdu.frame().sourceAddress(),
                    domainAddress);
                break;
            }
            case IndividualAddressSerialNumberWrite:
            {
                uint8_t* knxSerialNumber = &data[1];
                uint16_t newIndividualAddress;
                popWord(newIndividualAddress, &data[7]);
                _bau.individualAddressSerialNumberWriteIndication(priority, hopType, newIndividualAddress, knxSerialNumber);
                break;
            }
#if !((MEDIUM_TYPE == 5) || (MEDIUM_TYPE == 0))
            default:
                print("Broadcast-indication: unhandled APDU-Type: ");
                println(apdu.type());
                break;
        }
    }
    else if (broadcastType == SysBroadcast)
    {
        // APCI on SystemBroadcast
        switch (apdu.type())
        {
#endif
            // TODO: testInfo could be of any length
            case SystemNetworkParameterRead:
            {
                uint16_t objectType;
                uint16_t propertyId;
                uint8_t testInfo[2];
                popWord(objectType, data + 1);
                popWord(propertyId, data + 3);
                popByte(testInfo[0], data + 4);
                popByte(testInfo[1], data + 5);
                propertyId = (propertyId >> 4) & 0x0FFF;;
                testInfo[0] &= 0x0F;
                _bau.systemNetworkParameterReadIndication(priority, hopType, objectType, propertyId, testInfo, sizeof(testInfo));
                break;
            }
            case DomainAddressSerialNumberWrite:
            {
                const uint8_t* knxSerialNumber = &data[1];
                const uint8_t* domainAddress = &data[7];
                _bau.domainAddressSerialNumberWriteIndication(priority, hopType, domainAddress, knxSerialNumber);
                break;
            }
            case DomainAddressSerialNumberRead:
            {
                const uint8_t* knxSerialNumber = &data[1];
                _bau.domainAddressSerialNumberReadIndication(priority, hopType, knxSerialNumber);
                break;
            }
#if !((MEDIUM_TYPE == 5) || (MEDIUM_TYPE == 0))
            default:
                if (broadcastType == SysBroadcast)
                {
                    print("System");
                }
#endif
                print("Broadcast-indication: unhandled APDU-Type: ");
                println(apdu.type());
                break;
        }
    }
}

void ApplicationLayer::dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status, SystemBroadcast broadcastType)
{
    uint8_t* data = apdu.data();

    if (broadcastType == Broadcast)
    {
        // APCI on Broadcast
        switch (apdu.type())
        {
            case IndividualAddressWrite:
            {
                uint16_t newAddress;
                popWord(newAddress, data + 1);
                _bau.individualAddressWriteLocalConfirm(ack, hopType, newAddress, status);
                break;
            }
            case IndividualAddressRead:
                _bau.individualAddressReadLocalConfirm(ack, hopType, status);
                break;
            case IndividualAddressResponse:
                _bau.individualAddressReadResponseConfirm(ack, hopType, status);
                break;
            case IndividualAddressSerialNumberRead:
                _bau.individualAddressSerialNumberReadLocalConfirm(ack, hopType, data + 1, status);
                break;
            case IndividualAddressSerialNumberResponse:
            {
                uint16_t domainAddress;
                popWord(domainAddress, data + 7);
                _bau.individualAddressSerialNumberReadResponseConfirm(ack, hopType, data + 1, domainAddress, status);
                break;
            }
            case IndividualAddressSerialNumberWrite:
            {
                uint16_t newAddress;
                popWord(newAddress, data + 7);
                _bau.individualAddressSerialNumberWriteLocalConfirm(ack, hopType, data + 1, newAddress, status);
                break;
            }
#if !((MEDIUM_TYPE == 5) || (MEDIUM_TYPE == 0))
            default:
                print("Broadcast-confirm: unhandled APDU-Type: ");
                println(apdu.type());
                break;
        }
    }
    else if (broadcastType == SysBroadcast)
    {
        // APCI on SystemBroadcast
        switch (apdu.type())
        {
#endif
        case SystemNetworkParameterRead:
        {
            uint16_t objectType;
            uint16_t propertyId;
            uint8_t testInfo[2];
            popWord(objectType, data + 1);
            popWord(propertyId, data + 3);
            popByte(testInfo[0], data + 4);
            popByte(testInfo[1], data + 5);
            propertyId = (propertyId >> 4) & 0x0FFF;;
            testInfo[0] &= 0x0F;
            //TODO: _bau.systemNetworkParameterReadLocalConfirm(priority, hopType, objectType, propertyId, testInfo, sizeof(testInfo), status);
            break;
        }
        case DomainAddressSerialNumberWrite:
        {
            const uint8_t* knxSerialNumber = &data[1];
            const uint8_t* domainAddress = &data[7];
            //TODO: _bau.domainAddressSerialNumberWriteLocalConfirm(priority, hopType, domainAddress, knxSerialNumber, status);
            break;
        }
        case DomainAddressSerialNumberRead:
        {
            const uint8_t* knxSerialNumber = &data[1];
            //TODO: _bau.domainAddressSerialNumberReadLocalConfirm(priority, hopType, knxSerialNumber, status);
            break;
        }
        default:
#if !((MEDIUM_TYPE == 5) || (MEDIUM_TYPE == 0))
                if (broadcastType == SysBroadcast)
                {
                    print("System");
                }
#endif
            print("Broadcast-confirm: unhandled APDU-Type: ");
            println(apdu.type());
            break;
        }
    }
}

void ApplicationLayer::dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    individualIndication(hopType, priority, tsap, apdu);
}

void ApplicationLayer::dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status)
{
    individualConfirm(ack, hopType, priority, tsap, apdu, status);
}

void ApplicationLayer::connectIndication(uint16_t tsap)
{
    _connectedTsap = tsap;
}

void ApplicationLayer::connectConfirm(uint16_t destination, uint16_t tsap, bool status)
{
    if (status)
    {
        _connectedTsap = tsap;
        _bau.connectConfirm(tsap);
    }
    else
        _connectedTsap = -1;
}

void ApplicationLayer::disconnectIndication(uint16_t tsap)
{
    _connectedTsap = -1;
}

void ApplicationLayer::disconnectConfirm(Priority priority, uint16_t tsap, bool status)
{
    _connectedTsap = -1;
}

void ApplicationLayer::dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu)
{
    individualIndication(NetworkLayerParameter, priority, tsap, apdu);
}

void ApplicationLayer::dataConnectedConfirm(uint16_t tsap)
{

}
#pragma endregion
void ApplicationLayer::groupValueReadRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType)
{
    _savedAsapReadRequest = asap;
    CemiFrame frame(1);
    APDU& apdu = frame.apdu();
    apdu.type(GroupValueRead);
    
    int32_t value = _assocTable.translateAsap(asap);
    if (value < 0)
        return; // there is no tsap in association table for this asap
    
    uint16_t tsap = (uint16_t)value;

    // first to bus then to itself
    dataGroupRequest(ack, hopType, priority, tsap, apdu);
    dataGroupIndication(hopType, priority, tsap, apdu);
}

void ApplicationLayer::groupValueReadResponse(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength)
{
    _savedAsapResponse = asap;
    groupValueSend(GroupValueResponse, ack, asap, priority, hopType, data, dataLength);
}

void ApplicationLayer::groupValueWriteRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t * data, uint8_t dataLength)
{
    _savedAsapWriteRequest = asap;
    groupValueSend(GroupValueWrite, ack, asap, priority, hopType, data, dataLength);
}

void ApplicationLayer::individualAddressWriteRequest(AckType ack, HopCountType hopType, uint16_t newaddress)
{
    CemiFrame frame(3);
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressWrite);
    uint8_t* apduData = apdu.data();
    pushWord(newaddress, apduData + 1);
    dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void ApplicationLayer::individualAddressReadRequest(AckType ack, HopCountType hopType)
{
    CemiFrame frame(1);
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressRead);
    dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void ApplicationLayer::individualAddressReadResponse(AckType ack, HopCountType hopType)
{
    CemiFrame frame(1);
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressResponse);
    dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void ApplicationLayer::individualAddressSerialNumberReadRequest(AckType ack, HopCountType hopType, uint8_t * serialNumber)
{
    CemiFrame frame(7);
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressSerialNumberRead);
    uint8_t* data = apdu.data() + 1;
    memcpy(data, serialNumber, 6);
    dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void ApplicationLayer::individualAddressSerialNumberReadResponse(AckType ack, HopCountType hopType, 
    uint8_t * serialNumber, uint16_t domainAddress)
{
    CemiFrame frame(7);
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressSerialNumberResponse);
    uint8_t* data = apdu.data() + 1;
    memcpy(data, serialNumber, 6);
    data += 6;
    pushWord(domainAddress, data);
    dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void ApplicationLayer::individualAddressSerialNumberWriteRequest(AckType ack, HopCountType hopType, uint8_t * serialNumber,
    uint16_t newaddress)
{
    CemiFrame frame(13);
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressSerialNumberWrite);
    uint8_t* data = apdu.data() + 1;
    memcpy(data, serialNumber, 6);
    data += 6;
    pushWord(newaddress, data);
    dataBroadcastRequest(ack, hopType, SystemPriority, apdu);
}

void ApplicationLayer::deviceDescriptorReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t descriptorType)
{
    CemiFrame frame(1);
    APDU& apdu = frame.apdu();
    apdu.type(DeviceDescriptorRead);
    uint8_t* data = apdu.data();
    *data |= (descriptorType & 0x3f);
    
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::deviceDescriptorReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t descriptorType, uint8_t* deviceDescriptor)
{
    uint8_t length = 0;
    switch (descriptorType)
    {
    case 0:
        length = 3;
        break;
    case 2:
        length = 14;
        break;
    default:
        length = 1;
        descriptorType = 0x3f;
        break;
    }
    CemiFrame frame(length);
    APDU& apdu = frame.apdu();
    apdu.type(DeviceDescriptorResponse);
    uint8_t* data = apdu.data();
    *data |= (descriptorType & 0x3f);
    
    if (length > 1)
        memcpy(data + 1, deviceDescriptor, length - 1);
    
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::connectRequest(uint16_t destination, Priority priority)
{
    _transportLayer->connectRequest(destination, priority);
}

void ApplicationLayer::disconnectRequest(Priority priority)
{
    _transportLayer->disconnectRequest(_connectedTsap, priority);
}

void ApplicationLayer::restartRequest(AckType ack, Priority priority, HopCountType hopType)
{
    CemiFrame frame(1);
    APDU& apdu = frame.apdu();
    apdu.type(Restart);

    individualSend(ack, hopType, priority, _connectedTsap, apdu);
}

//TODO: ApplicationLayer::systemNetworkParameterReadRequest()
void ApplicationLayer::systemNetworkParameterReadResponse(Priority priority, HopCountType hopType, 
                                                          uint16_t objectType, uint16_t propertyId,
                                                          uint8_t* testInfo, uint16_t testInfoLength, 
                                                          uint8_t* testResult, uint16_t testResultLength)
{
    CemiFrame frame(testInfoLength + testResultLength + 3 + 1); // PID and testInfo share an octet (+3) and +1 for APCI byte(?)
    APDU& apdu = frame.apdu();
    apdu.type(SystemNetworkParameterResponse);
    uint8_t* data = apdu.data() + 1;

    pushWord(objectType, data);
    pushWord((propertyId << 4) & 0xFFF0, data + 2);                             // Reserved bits for test_info are always 0
    uint8_t* pData = pushByteArray(&testInfo[1], testInfoLength - 1, data + 4); // TODO: upper reserved bits (testInfo + 0) have to put into the lower bits of data + 3
    memcpy(pData, testResult, testResultLength);

    //apdu.printPDU();

    dataSystemBroadcastRequest(AckDontCare, hopType, SystemPriority, apdu);
}

//TODO: ApplicationLayer::domainAddressSerialNumberWriteRequest()
//TODO: ApplicationLayer::domainAddressSerialNumberReadRequest()
void ApplicationLayer::domainAddressSerialNumberReadResponse(Priority priority, HopCountType hopType, uint8_t* rfDoA,
                                                             const uint8_t* knxSerialNumber)
{
    CemiFrame frame(13); 
    APDU& apdu = frame.apdu();
    apdu.type(DomainAddressSerialNumberResponse);

    uint8_t* data = apdu.data() + 1;

    memcpy(data, knxSerialNumber, 6);
    memcpy(data + 6, rfDoA, 6);

    //apdu.printPDU();

    dataSystemBroadcastRequest(AckDontCare, hopType, SystemPriority, apdu);
}

//TODO: ApplicationLayer::IndividualAddressSerialNumberWriteRequest()
//TODO: ApplicationLayer::IndividualAddressSerialNumberReadRequest()
void ApplicationLayer::IndividualAddressSerialNumberReadResponse(Priority priority, HopCountType hopType, uint8_t* rfDoA,
                                                                 uint8_t* knxSerialNumber)
{
    CemiFrame frame(13); 
    APDU& apdu = frame.apdu();
    apdu.type(IndividualAddressSerialNumberResponse);

    uint8_t* data = apdu.data() + 1;

    memcpy(data, knxSerialNumber, 6);
    memcpy(data + 6, rfDoA, 6);

     //apdu.printPDU();

    dataBroadcastRequest(AckDontCare, hopType, SystemPriority, apdu);
}

void ApplicationLayer::propertyValueReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
    CemiFrame frame(5);
    APDU& apdu = frame.apdu();
    apdu.type(PropertyValueRead);
    uint8_t* data = apdu.data();
    data += 1;
    data = pushByte(objectIndex, data);
    data = pushByte(propertyId, data);
    pushWord(startIndex & 0xfff, data);
    *data &= ((numberOfElements & 0xf) << 4);
    
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::propertyValueReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    propertyDataSend(PropertyValueResponse, ack, priority, hopType, asap, objectIndex, propertyId, numberOfElements,
        startIndex, data, length);
}

void ApplicationLayer::propertyValueWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t * data, uint8_t length)
{
    propertyDataSend(PropertyValueWrite, ack, priority, hopType, asap, objectIndex, propertyId, numberOfElements,
        startIndex, data, length);
}

void ApplicationLayer::functionPropertyStateResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                                     uint8_t objectIndex, uint8_t propertyId, uint8_t* resultData, uint8_t resultLength)
{
    CemiFrame frame(3 + resultLength + 1);
    APDU& apdu = frame.apdu();
    apdu.type(FunctionPropertyStateResponse);
    uint8_t* data = apdu.data() + 1;

    data[0] = objectIndex;
    data[1] = propertyId;
    if (resultLength > 0)
        memcpy(&data[2], resultData, resultLength);

    if (asap == _connectedTsap)
        dataConnectedRequest(asap, priority, apdu);
    else
        dataIndividualRequest(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::propertyDescriptionReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex)
{
    CemiFrame frame(4);
    APDU& apdu = frame.apdu();
    apdu.type(PropertyDescriptionRead);
    uint8_t* data = apdu.data();
    data[1] = objectIndex;
    data[2] = propertyId;
    data[3] = propertyIndex;
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type, 
    uint16_t maxNumberOfElements, uint8_t access)
{
    CemiFrame frame(8);
    APDU& apdu = frame.apdu();
    apdu.type(PropertyDescriptionResponse);
    uint8_t* data = apdu.data();
    data[1] = objectIndex;
    data[2] = propertyId;
    data[3] = propertyIndex;
    if (writeEnable)
        data[4] |= 0x80;
    data[4] |= (type & 0x3f);
    pushWord(maxNumberOfElements & 0xfff, data + 5);
    data[7] = access;
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::memoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress)
{
    CemiFrame frame(3);
    APDU& apdu = frame.apdu();
    apdu.type(MemoryRead);
    uint8_t* data = apdu.data();
    *data |= (number & 0x3f);
    pushWord(memoryAddress, data + 1);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::memoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress, uint8_t * memoryData)
{
    memorySend(MemoryResponse, ack, priority, hopType, asap, number, memoryAddress, memoryData);
}

void ApplicationLayer::memoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t number, uint16_t memoryAddress, uint8_t * data)
{
    memorySend(MemoryWrite, ack, priority, hopType, asap, number, memoryAddress, data);
}

void ApplicationLayer::userMemoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t number, uint32_t memoryAddress)
{
    CemiFrame frame(4);
    APDU& apdu = frame.apdu();
    apdu.type(UserMemoryRead);
    uint8_t* data = apdu.data();
    data[1] |= (number & 0xf);
    data[1] |= ((memoryAddress >> 12) & 0xf0);
    pushWord(memoryAddress & 0xff, data + 2);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::userMemoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t number, uint32_t memoryAddress, uint8_t * memoryData)
{
    userMemorySend(UserMemoryResponse, ack, priority, hopType, asap, number, memoryAddress, memoryData);
}

void ApplicationLayer::userMemoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t number, uint32_t memoryAddress, uint8_t * memoryData)
{
    userMemorySend(UserMemoryWrite, ack, priority, hopType, asap, number, memoryAddress, memoryData);
}

void ApplicationLayer::userManufacturerInfoReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap)
{
    CemiFrame frame(1);
    APDU& apdu = frame.apdu();
    apdu.type(UserManufacturerInfoRead);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::userManufacturerInfoReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t* info)
{
    CemiFrame frame(4);
    APDU& apdu = frame.apdu();
    apdu.type(UserMemoryRead);
    uint8_t* data = apdu.data();
    memcpy(data + 1, info, 3);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::authorizeRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint32_t key)
{
    CemiFrame frame(6);
    APDU& apdu = frame.apdu();
    apdu.type(AuthorizeRequest);
    uint8_t* data = apdu.data();
    pushInt(key, data + 2);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::authorizeResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level)
{
    CemiFrame frame(2);
    APDU& apdu = frame.apdu();
    apdu.type(AuthorizeResponse);
    uint8_t* data = apdu.data();
    data[1] = level;
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::keyWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level, uint32_t key)
{
    CemiFrame frame(6);
    APDU& apdu = frame.apdu();
    apdu.type(KeyWrite);
    uint8_t* data = apdu.data();
    data[1] = level;
    pushInt(key, data + 2);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::keyWriteResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level)
{
    CemiFrame frame(6);
    APDU& apdu = frame.apdu();
    apdu.type(KeyResponse);
    uint8_t* data = apdu.data();
    data[1] = level;
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    CemiFrame frame(5 + length);
    APDU& apdu = frame.apdu();
    apdu.type(type);
    uint8_t* apduData = apdu.data();
    apduData += 1;
    apduData = pushByte(objectIndex, apduData);
    apduData = pushByte(propertyId, apduData);
    pushWord(startIndex & 0xfff, apduData);
    *apduData |= ((numberOfElements & 0xf) << 4);
    apduData += 2;
    if (length > 0)
        memcpy(apduData, data, length);

    if (asap == _connectedTsap)
        dataConnectedRequest(asap, priority, apdu);
    else
        dataIndividualRequest(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::groupValueSend(ApduType type, AckType ack, uint16_t asap, Priority priority, HopCountType hopType, 
    uint8_t* data,  uint8_t& dataLength)
{
    CemiFrame frame(dataLength + 1);
    APDU& apdu = frame.apdu();
    apdu.type(type);
    uint8_t* apdudata = apdu.data();
    if (dataLength == 0)
    {
        // data size is six bit or less. So store int first byte
        *apdudata &= ~0x3f;
        *apdudata |= (*data & 0x3f);
    }
    else
    {
        memcpy(apdudata + 1, data, dataLength);
    }
    // no need to check if there is a tsap. This is a response, so the read got trough
    uint16_t tsap = (uint16_t)_assocTable.translateAsap(asap);
    dataGroupRequest(ack, hopType, priority, tsap, apdu);
    dataGroupIndication(hopType, priority, tsap, apdu);
}

void ApplicationLayer::memorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint16_t memoryAddress, uint8_t * memoryData)
{
    CemiFrame frame(3 + number);
    APDU& apdu = frame.apdu();
    apdu.type(type);
    uint8_t* data = apdu.data();
    *data |= (number & 0x3f);
    pushWord(memoryAddress, data + 1);
    if (number > 0)
        memcpy(data + 3, memoryData, number);

    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::userMemorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
    uint32_t memoryAddress, uint8_t * memoryData)
{
    CemiFrame frame(4 + number);
    APDU& apdu = frame.apdu();
    apdu.type(type);
    uint8_t* data = apdu.data();
    data[1] |= (number & 0xf);
    data[1] |= ((memoryAddress >> 12) & 0xf0);
    pushWord(memoryAddress & 0xffff, data + 2);
    if (number > 0)
        memcpy(data + 4, memoryData, number);
    individualSend(ack, hopType, priority, asap, apdu);
}

void ApplicationLayer::individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU & apdu)
{
    uint8_t* data = apdu.data();
    switch (apdu.type())
    {
        case DeviceDescriptorRead:
            _bau.deviceDescriptorReadIndication(priority, hopType, tsap, *data & 0x3f);
            break;
        case DeviceDescriptorResponse:
            _bau.deviceDescriptorReadAppLayerConfirm(priority, hopType, tsap, *data & 0x3f, data + 1);
            break;
        case Restart:
            if ((*data & 0x3f) == 0)
                _bau.restartRequestIndication(priority, hopType, tsap);
            break;
        case PropertyValueRead:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadIndication(priority, hopType, tsap, data[1], data[2], data[3] >> 4, startIndex);
            break;
        }
        case PropertyValueResponse:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadAppLayerConfirm(priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5);
            break;
        }
        case PropertyValueWrite:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueWriteIndication(priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5);
            break;
        }
        case FunctionPropertyCommand:
            _bau.functionPropertyCommandIndication(priority, hopType, tsap, data[1], data[2], &data[3], apdu.length() - 4); //TODO: check length
            break;
        case FunctionPropertyState:
            _bau.functionPropertyStateIndication(priority, hopType, tsap, data[1], data[2], &data[3], apdu.length() - 4); //TODO: check length
            break;
        case PropertyDescriptionRead:
            _bau.propertyDescriptionReadIndication(priority, hopType, tsap, data[1], data[2], data[3]);
            break;
        case PropertyDescriptionResponse:
            _bau.propertyDescriptionReadAppLayerConfirm(priority, hopType, tsap, data[1], data[2], data[3],
                (data[4] & 0x80) > 0, data[4] & 0x3f, getWord(data + 5) & 0xfff, data[7]);
            break;
        case MemoryRead:
            _bau.memoryReadIndication(priority, hopType, tsap, data[0] & 0x3f, getWord(data + 1));
            break;
        case MemoryResponse:
            _bau.memoryReadAppLayerConfirm(priority, hopType, tsap, data[0] & 0x3f, getWord(data + 1), data + 3);
            break;
        case MemoryWrite:
            _bau.memoryWriteIndication(priority, hopType, tsap, data[0] & 0x3f, getWord(data + 1), data + 3);
            break;
        case UserMemoryRead:
        {
            uint32_t address = ((data[1] & 0xf0) << 12) + (data[2] << 8) + data[3];
            _bau.userMemoryReadIndication(priority, hopType, tsap, data[1] & 0xf, address);
            break;
        }
        case UserMemoryResponse:
        {
            uint32_t address = ((data[1] & 0xf0) << 12) + (data[2] << 8) + data[3];
            _bau.userMemoryReadAppLayerConfirm(priority, hopType, tsap, data[1] & 0xf, address, data + 4);
            break;
        }
        case UserMemoryWrite:
        {
            uint32_t address = ((data[1] & 0xf0) << 12) + (data[2] << 8) + data[3];
            _bau.userMemoryWriteIndication(priority, hopType, tsap, data[1] & 0xf, address, data + 4);
            break;
        }
        case UserManufacturerInfoRead:
            _bau.userManufacturerInfoIndication(priority, hopType, tsap);
            break;
        case UserManufacturerInfoResponse:
            _bau.userManufacturerInfoAppLayerConfirm(priority, hopType, tsap, data + 1);
            break;
        case AuthorizeRequest:
            _bau.authorizeIndication(priority, hopType, tsap, getInt(data + 2));
            break;
        case AuthorizeResponse:
            _bau.authorizeAppLayerConfirm(priority, hopType, tsap, data[1]);
            break;
        case KeyWrite:
            _bau.keyWriteIndication(priority, hopType, tsap, data[1], getInt(data + 2));
            break;
        case KeyResponse:
            _bau.keyWriteAppLayerConfirm(priority, hopType, tsap, data[1]);
            break;
        default:
            print("Indiviual-indication: unhandled APDU-Type: ");
            println(apdu.type());
    }
}

void ApplicationLayer::individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU & apdu, bool status)
{
    uint8_t* data = apdu.data();
    switch (apdu.type())
    {
        case DeviceDescriptorRead:
            _bau.deviceDescriptorReadLocalConfirm(ack, priority, hopType, tsap, *data & 0x3f, status);
            break;
        case DeviceDescriptorResponse:
            _bau.deviceDescriptorReadResponseConfirm(ack, priority, hopType, tsap, *data & 0x3f, data + 1, status);
            break;
        case Restart:
            _bau.restartRequestLocalConfirm(ack, priority, hopType, tsap, status);
            break;
        case PropertyValueRead:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadLocalConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, status);
            break;
        }
        case PropertyValueResponse:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadResponseConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5, status);
            break;
        }
        case PropertyValueWrite:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueWriteLocalConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5, status);
            break;
        }
        case PropertyDescriptionRead:
            _bau.propertyDescriptionReadLocalConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3], status);
            break;
        case PropertyDescriptionResponse:
            _bau.propertyDescriptionReadResponseConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3],
                (data[4] & 0x80) > 0, data[4] & 0x3f, getWord(data + 5) & 0xfff, data[7], status);
            break;
        case MemoryRead:
            _bau.memoryReadLocalConfirm(ack, priority, hopType, tsap, data[0] & 0x3f, getWord(data + 1), status);
            break;
        case MemoryResponse:
            _bau.memoryReadResponseConfirm(ack, priority, hopType, tsap, data[0] & 0x3f, getWord(data + 1), data + 3, status);
            break;
        case MemoryWrite:
            _bau.memoryWriteLocalConfirm(ack, priority, hopType, tsap, data[0] & 0x3f, getWord(data + 1), data + 3, status);
            break;
        case UserMemoryRead:
        {
            uint32_t address = ((data[1] & 0xf0) << 12) + (data[2] << 8) + data[3];
            _bau.memoryReadLocalConfirm(ack, priority, hopType, tsap, data[1] & 0xf, address, status);
            break;
        }
        case UserMemoryResponse:
        {
            uint32_t address = ((data[1] & 0xf0) << 12) + (data[2] << 8) + data[3];
            _bau.memoryReadResponseConfirm(ack, priority, hopType, tsap, data[1] & 0xf, address, data + 4, status);
            break;
        }
        case UserMemoryWrite:
        {
            uint32_t address = ((data[1] & 0xf0) << 12) + (data[2] << 8) + data[3];
            _bau.memoryWriteLocalConfirm(ack, priority, hopType, tsap, data[1] & 0xf, address, data + 4, status);
            break;
        }        
        case UserManufacturerInfoRead:
            _bau.userManufacturerInfoLocalConfirm(ack, priority, hopType, tsap, status);
            break;
        case UserManufacturerInfoResponse:
            _bau.userManufacturerInfoResponseConfirm(ack, priority, hopType, tsap, data + 1, status);
            break;
        case AuthorizeRequest:
            _bau.authorizeLocalConfirm(ack, priority, hopType, tsap, getInt(data + 2), status);
            break;
        case AuthorizeResponse:
            _bau.authorizeResponseConfirm(ack, priority, hopType, tsap, data[1], status);
            break;
        case KeyWrite:
            _bau.keyWriteLocalConfirm(ack, priority, hopType, tsap, data[1], getInt(data + 2), status);
            break;
        case KeyResponse:
            _bau.keyWriteResponseConfirm(ack, priority, hopType, tsap, data[1], status);
            break;
        default:
            print("Indiviual-confirm: unhandled APDU-Type: ");
            println(apdu.type());
    }
}

void ApplicationLayer::individualSend(AckType ack, HopCountType hopType, Priority priority, uint16_t asap, APDU& apdu)
{
    if (asap == _connectedTsap)
        dataConnectedRequest(asap, priority, apdu);
    else
        dataIndividualRequest(ack, hopType, priority, asap, apdu);
}

bool ApplicationLayer::isConnected()
{
    return (_connectedTsap >= 0);
}

void ApplicationLayer::dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu)
{
    _transportLayer->dataGroupRequest(ack, hopType, priority, tsap, apdu);
}
void ApplicationLayer::dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    _transportLayer->dataBroadcastRequest(ack, hopType, SystemPriority, apdu, Broadcast);
}
void ApplicationLayer::dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu)
{
    _transportLayer->dataBroadcastRequest(ack, hopType, SystemPriority, apdu, SysBroadcast);
}
void ApplicationLayer::dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu)
{
    _transportLayer->dataIndividualRequest(ack, hopType, priority, destination, apdu);
}
void ApplicationLayer::dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu)
{
    // apdu must be valid until it was confirmed
    _transportLayer->dataConnectedRequest(tsap, priority, apdu);
}
