#include "bau.h"

void BusAccessUnit::groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl& secCtrl, bool status)
{
}

void BusAccessUnit::groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl)
{
}

void BusAccessUnit::groupValueReadResponseConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopTtype, const SecurityControl &secCtrl, uint8_t* data, uint8_t dataLength, bool status)
{
}

void BusAccessUnit::groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* data, uint8_t dataLength)
{
}

void BusAccessUnit::groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* data, uint8_t dataLength, bool status)
{
}

void BusAccessUnit::groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* data, uint8_t dataLength)
{
}

void BusAccessUnit::individualAddressWriteLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, uint16_t newaddress, bool status)
{
}

void BusAccessUnit::individualAddressWriteIndication(HopCountType hopType, const SecurityControl &secCtrl, uint16_t newaddress)
{
}

void BusAccessUnit::individualAddressReadLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, bool status)
{
}

void BusAccessUnit::individualAddressReadIndication(HopCountType hopType, const SecurityControl &secCtrl)
{
}

void BusAccessUnit::individualAddressReadResponseConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, bool status)
{
}

void BusAccessUnit::individualAddressReadAppLayerConfirm(HopCountType hopType, const SecurityControl &secCtrl, uint16_t individualAddress)
{
}

void BusAccessUnit::individualAddressSerialNumberReadLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* serialNumber, bool status)
{
}

void BusAccessUnit::individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* knxSerialNumber)
{
}

void BusAccessUnit::individualAddressSerialNumberReadResponseConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* serialNumber, uint16_t domainAddress, bool status)
{
}

void BusAccessUnit::individualAddressSerialNumberReadAppLayerConfirm(HopCountType hopType, const SecurityControl &secCtrl, uint8_t* serialNumber, uint16_t individualAddress, uint16_t domainAddress)
{
}

void BusAccessUnit::individualAddressSerialNumberWriteLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* serialNumber, uint16_t newaddress, bool status)
{
}

void BusAccessUnit::individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t newIndividualAddress,
                                                                 uint8_t* knxSerialNumber)
{
}

void BusAccessUnit::deviceDescriptorReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptorType, bool status)
{
}

void BusAccessUnit::deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptorType)
{
}

void BusAccessUnit::deviceDescriptorReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptor_type,
                                                        uint8_t* device_descriptor, bool status)
{
}

void BusAccessUnit::deviceDescriptorReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptortype, uint8_t* deviceDescriptor)
{
}

void BusAccessUnit::restartRequestLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, bool status)
{
}

void BusAccessUnit::restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, RestartType restartType, EraseCode eraseCode, uint8_t channel)
{
}

void BusAccessUnit::propertyValueReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, bool status)
{
}

void BusAccessUnit::propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
}

void BusAccessUnit::propertyValueExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                   uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
}

void BusAccessUnit::functionPropertyCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t* data, uint8_t length)
{
}

void BusAccessUnit::functionPropertyStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t* data, uint8_t length)
{
}

void BusAccessUnit::functionPropertyExtCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance, uint8_t propertyId, uint8_t* data, uint8_t length)
{
}

void BusAccessUnit::functionPropertyExtStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance, uint8_t propertyId, uint8_t* data, uint8_t length)
{
}

void BusAccessUnit::propertyValueReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool status)
{
}

void BusAccessUnit::propertyValueReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
}

void BusAccessUnit::propertyValueWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool status)
{
}

void BusAccessUnit::propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
}

void BusAccessUnit::propertyValueExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool confirmed)
{
}

void BusAccessUnit::propertyDescriptionReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool status)
{
}

void BusAccessUnit::propertyExtDescriptionReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint16_t objectIndex, uint8_t propertyId, uint16_t propertyIndex, bool status)
{
}

void BusAccessUnit::propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex)
{
}

void BusAccessUnit::propertyExtDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
uint16_t objectType, uint16_t objectInstance, uint16_t propertyId, uint8_t descriptionType, uint16_t propertyIndex)
{
}

void BusAccessUnit::propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type, uint16_t maxNumberOfElements, uint8_t access)
{
}

void BusAccessUnit::propertyDescriptionReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type, uint16_t maxNumberOfElements, uint8_t access, bool status)
{
}

void BusAccessUnit::propertyDescriptionReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type, uint16_t maxNumberOfElements, uint8_t access)
{
}

void BusAccessUnit::memoryReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, bool status)
{
}

void BusAccessUnit::memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress)
{
}

void BusAccessUnit::memoryReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data, bool status)
{
}

void BusAccessUnit::memoryReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data)
{
}

void BusAccessUnit::memoryWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data, bool status)
{
}

void BusAccessUnit::memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data)
{
}

void BusAccessUnit::memoryRouterWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data)
{
}
void BusAccessUnit::memoryRouterReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data)
{
}
void BusAccessUnit::memoryRoutingTableReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress)
{
}
void BusAccessUnit::memoryRoutingTableReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t* data)
{
}
void BusAccessUnit::memoryRoutingTableWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t *data)
{
}

void BusAccessUnit::memoryExtReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, bool status)
{
}

void BusAccessUnit::memoryExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress)
{
}

void BusAccessUnit::memoryExtReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data, bool status)
{
}

void BusAccessUnit::memoryExtReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
}

void BusAccessUnit::memoryExtWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data, bool status)
{
}

void BusAccessUnit::memoryExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
}

void BusAccessUnit::memoryExtWriteResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data, bool status)
{
}

void BusAccessUnit::memoryExtWriteAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* data)
{
}

void BusAccessUnit::userMemoryReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, bool status)
{
}

void BusAccessUnit::userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress)
{
}

void BusAccessUnit::userMemoryReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* memoryData, bool status)
{
}

void BusAccessUnit::userMemoryReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* memoryData)
{
}

void BusAccessUnit::userMemoryWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* memoryData, bool status)
{
}

void BusAccessUnit::userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress, uint8_t* memoryData)
{
}

void BusAccessUnit::userManufacturerInfoLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, bool status)
{
}

void BusAccessUnit::userManufacturerInfoIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl)
{
}

void BusAccessUnit::userManufacturerInfoResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t* info, bool status)
{
}

void BusAccessUnit::userManufacturerInfoAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t* info)
{
}

void BusAccessUnit::authorizeLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint32_t key, bool status)
{
}

void BusAccessUnit::authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint32_t key)
{
}

void BusAccessUnit::authorizeResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level, bool status)
{
}

void BusAccessUnit::authorizeAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level)
{
}

void BusAccessUnit::keyWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level, uint32_t key, bool status)
{
}

void BusAccessUnit::keyWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level, uint32_t key)
{
}

void BusAccessUnit::keyWriteResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level, bool status)
{
}

void BusAccessUnit::keyWriteAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level)
{
}

void BusAccessUnit::connectConfirm(uint16_t destination)
{
}

void BusAccessUnit::systemNetworkParameterReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                         uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength)
{
}

void BusAccessUnit::domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                             const uint8_t* knxSerialNumber)
{
}

void BusAccessUnit::domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber)
{
}

void BusAccessUnit::systemNetworkParameterReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                         uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength, bool status)
{
}

void BusAccessUnit::domainAddressSerialNumberWriteLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                             const uint8_t* knxSerialNumber, bool status)
{
}

void BusAccessUnit::domainAddressSerialNumberReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber, bool status)
{
}

void BusAccessUnit::propertyValueRead(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                      uint8_t& numberOfElements, uint16_t startIndex,
                                      uint8_t** data, uint32_t& length)
{
}

void BusAccessUnit::propertyValueWrite(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                       uint8_t& numberOfElements, uint16_t startIndex,
                                       uint8_t* data, uint32_t length)
{
}

void BusAccessUnit::beforeRestartCallback(BeforeRestartCallback func)
{
}

BeforeRestartCallback BusAccessUnit::beforeRestartCallback()
{
    return 0;
}

void BusAccessUnit::functionPropertyCallback(FunctionPropertyCallback func)
{
}

FunctionPropertyCallback BusAccessUnit::functionPropertyCallback()
{
    return 0;
}

void BusAccessUnit::functionPropertyStateCallback(FunctionPropertyCallback func)
{
}

FunctionPropertyCallback BusAccessUnit::functionPropertyStateCallback()
{
    return 0;
}