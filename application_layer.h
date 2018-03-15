#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "apdu.h"

class AssociationTableObject;
class BusAccessUnit;
class TransportLayer;

class ApplicationLayer
{
public:
    ApplicationLayer(AssociationTableObject& assocTable, BusAccessUnit& bau);
    void transportLayer(TransportLayer& layer);

    // from transport layer
#pragma region Transport-Layer-Callbacks
    void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap,
        APDU& apdu, bool status);
    void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status);
    void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status);
    void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
    void connectIndication(uint16_t tsap);
    void connectConfirm(uint16_t destination, uint16_t tsap, bool status);
    void disconnectIndication(uint16_t tsap);
    void disconnectConfirm(Priority priority, uint16_t tsap, bool status);
    void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu);
    void dataConnectedConfirm(uint16_t tsap);
#pragma endregion

    void groupValueReadRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType);
    void groupValueReadResponse(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, uint8_t dataLength);
    void groupValueWriteRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, uint8_t dataLength);
    void individualAddressWriteRequest(AckType ack, HopCountType hopType, uint16_t newaddress);
    void individualAddressReadRequest(AckType ack, HopCountType hopType);
    void individualAddressReadResponse(AckType ack, HopCountType hopType);
    void individualAddressSerialNumberReadRequest(AckType ack, HopCountType hopType, uint8_t* serialNumber);
    void individualAddressSerialNumberReadResponse(AckType ack, HopCountType hopType, uint8_t* serialNumber,
        uint16_t domainAddress);
    void individualAddressSerialNumberWriteRequest(AckType ack, HopCountType hopType, uint8_t* serialNumber,
        uint16_t newaddress);
    void deviceDescriptorReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t descriptorType);
    void deviceDescriptorReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t descriptorType, uint8_t* deviceDescriptor);
    void restartRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap);
    void propertyValueReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    void propertyValueReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex, 
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
     void propertyValueWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
     void propertyDescriptionReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
         uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex);
     void propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
         uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
         uint16_t maxNumberOfElements, uint8_t access);
     void memoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
         uint16_t memoryAddress);
     void memoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
         uint16_t memoryAddress, uint8_t* data);
     void memoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
         uint16_t memoryAddress, uint8_t* data);
     void userMemoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
         uint32_t memoryAddress);
     void userMemoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
         uint32_t memoryAddress, uint8_t* memoryData);
     void userMemoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
         uint32_t memoryAddress, uint8_t* memoryData);
     void userManufacturerInfoReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap);
     void userManufacturerInfoReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
         uint8_t* info);
     void authorizeRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint32_t key);
     void authorizeResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level);
     void keyWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level, uint32_t key);
     void keyWriteResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level);
private:
    void propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
        uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t * data, 
        uint8_t length);
    void memorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, uint8_t * memoryData);
    void userMemorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t number, uint32_t memoryAddress, uint8_t* memoryData);
    void groupValueSend(ApduType type, AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, uint8_t& dataLength);
    void individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
    void individualSend(AckType ack, HopCountType hopType, Priority priority, uint16_t asap, APDU& apdu);

    uint16_t _savedAsapReadRequest;
    uint16_t _savedAsapWriteRequest;
    uint16_t _savedAsapResponse;
    AssociationTableObject& _assocTable;
    BusAccessUnit& _bau;
    TransportLayer* _transportLayer;
    int32_t _connectedTsap;
};
