#pragma once
#include <stdint.h>
#include "knx_types.h"

class BusAccessUnit
{
public:
    virtual void groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, bool status);
    virtual void groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType);
    virtual void groupValueReadResponseConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopTtype,
        uint8_t* data, uint8_t dataLength, bool status);
    virtual void groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType,
        uint8_t* data, uint8_t dataLength);
    virtual void groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType,
        uint8_t* data, uint8_t dataLength, bool status);
    virtual void groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType,
        uint8_t* data, uint8_t dataLength);
    virtual void individualAddressWriteLocalConfirm(AckType ack, HopCountType hopType,
        uint16_t newaddress, bool status);
    virtual void individualAddressWriteIndication(HopCountType hopType, uint16_t newaddress);
    virtual void individualAddressReadLocalConfirm(AckType ack, HopCountType hopType, bool status);
    virtual void individualAddressReadIndication(HopCountType hopType);
    virtual void individualAddressReadResponseConfirm(AckType ack, HopCountType hopType, bool status);
    virtual void individualAddressReadAppLayerConfirm(HopCountType hopType, uint16_t individualAddress);
    virtual void individualAddressSerialNumberReadLocalConfirm(AckType ack, HopCountType hopType,
        uint8_t* serialNumber, bool status);
    virtual void individualAddressSerialNumberReadIndication(HopCountType hopType, uint8_t* serialNumber);
    virtual void individualAddressSerialNumberReadResponseConfirm(AckType ack, HopCountType hopType,
        uint8_t* serialNumber, uint16_t domainAddress, bool status);
    virtual void individualAddressSerialNumberReadAppLayerConfirm(HopCountType hopType, uint8_t* serialNumber,
        uint16_t individualAddress, uint16_t domainAddress);
    virtual void individualAddressSerialNumberWriteLocalConfirm(AckType ack, HopCountType hopType, uint8_t* serialNumber,
        uint16_t newaddress, bool status);
    virtual void individualAddressSerialNumberWriteIndication(HopCountType hopType, uint8_t* serialNumber, uint16_t newaddress);
    virtual void deviceDescriptorReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t descriptorType, bool status);
    virtual void deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t descriptorType);
    virtual void deviceDescriptorReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t descriptor_type, uint8_t* device_descriptor, bool status);
    virtual void deviceDescriptorReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t descriptortype, uint8_t* deviceDescriptor);
    virtual void restartRequestLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, bool status);
    virtual void restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap);
    virtual void propertyValueReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, bool status);
    virtual void propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    virtual void propertyValueReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool status);
    virtual void propertyValueReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    virtual void propertyValueWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool status);
    virtual void propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    virtual void propertyDescriptionReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool status);
    virtual void propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex);
    virtual void propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
        uint16_t maxNumberOfElements, uint8_t access);
    virtual void propertyDescriptionReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
        uint16_t maxNumberOfElements, uint8_t access, bool status);
    virtual void propertyDescriptionReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
        uint16_t maxNumberOfElements, uint8_t access);
    virtual void memoryReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, bool status);
    virtual void memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint16_t memoryAddress);
    virtual void memoryReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, uint8_t* data);
    virtual void memoryWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, uint8_t* data);
    virtual void userMemoryReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint32_t memoryAddress, bool status);
    virtual void userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint32_t memoryAddress);
    virtual void userMemoryReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint32_t memoryAddress, uint8_t* memoryData, bool status);
    virtual void userMemoryReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint32_t memoryAddress, uint8_t* memoryData);
    virtual void userMemoryWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint32_t memoryAddress, uint8_t* memoryData, bool status);
    virtual void userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint32_t memoryAddress, uint8_t* memoryData);
    virtual void userManufacturerInfoLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, bool status);
    virtual void userManufacturerInfoIndication(Priority priority, HopCountType hopType, uint16_t asap);
    virtual void userManufacturerInfoResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t* info, bool status);
    virtual void userManufacturerInfoAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap,
        uint8_t* info);
    virtual void authorizeLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint32_t key, bool status);
    virtual void authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, uint32_t key);
    virtual void authorizeResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level,
        bool status);
    virtual void authorizeAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, uint8_t level);
    virtual void keyWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level,
        uint32_t key, bool status);
    virtual void keyWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t level,
        uint32_t key);
    virtual void keyWriteResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level,
        bool status);
    virtual void keyWriteAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, uint8_t level);
};
