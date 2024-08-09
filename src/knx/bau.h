#pragma once
#include <stdint.h>
#include "knx_types.h"
#include "interface_object.h"

typedef void (*BeforeRestartCallback)(void);
typedef bool (*FunctionPropertyCallback)(uint8_t objectIndex, uint8_t propertyId, uint8_t length, uint8_t *data, uint8_t *resultData, uint8_t &resultLength);

class BusAccessUnit
{
  public:
    virtual ~BusAccessUnit() {}
    virtual void groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, bool status);
    virtual void groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl);
    virtual void groupValueReadResponseConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopTtype, const SecurityControl &secCtrl,
                                               uint8_t* data, uint8_t dataLength, bool status);
    virtual void groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                               uint8_t* data, uint8_t dataLength);
    virtual void groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                             uint8_t* data, uint8_t dataLength, bool status);
    virtual void groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                           uint8_t* data, uint8_t dataLength);
    virtual void individualAddressWriteLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl,
                                                    uint16_t newaddress, bool status);
    virtual void individualAddressWriteIndication(HopCountType hopType, const SecurityControl &secCtrl, uint16_t newaddress);
    virtual void individualAddressReadLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, bool status);
    virtual void individualAddressReadIndication(HopCountType hopType, const SecurityControl &secCtrl);
    virtual void individualAddressReadResponseConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, bool status);
    virtual void individualAddressReadAppLayerConfirm(HopCountType hopType, const SecurityControl &secCtrl, uint16_t individualAddress);
    virtual void individualAddressSerialNumberReadLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl,
                                                               uint8_t* serialNumber, bool status);
    virtual void individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* knxSerialNumber);
    virtual void individualAddressSerialNumberReadResponseConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl,
                                                                  uint8_t* serialNumber, uint16_t domainAddress, bool status);
    virtual void individualAddressSerialNumberReadAppLayerConfirm(HopCountType hopType, const SecurityControl &secCtrl, uint8_t* serialNumber,
                                                                  uint16_t individualAddress, uint16_t domainAddress);
    virtual void individualAddressSerialNumberWriteLocalConfirm(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* serialNumber,
                                                                uint16_t newaddress, bool status);
    virtual void individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t newIndividualAddress,
                                                              uint8_t* knxSerialNumber);
    virtual void deviceDescriptorReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                  uint8_t descriptorType, bool status);
    virtual void deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptorType);
    virtual void deviceDescriptorReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                     uint8_t descriptor_type, uint8_t* device_descriptor, bool status);
    virtual void deviceDescriptorReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                     uint8_t descriptortype, uint8_t* deviceDescriptor);
    virtual void restartRequestLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, bool status);
    virtual void restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, RestartType restartType, EraseCode eraseCode, uint8_t channel);
    virtual void propertyValueReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                               uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, bool status);
    virtual void propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                             uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    virtual void propertyValueExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    virtual void functionPropertyCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                   uint8_t propertyId, uint8_t* data, uint8_t length);
    virtual void functionPropertyStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                 uint8_t propertyId, uint8_t* data, uint8_t length);
    virtual void functionPropertyExtCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                      uint8_t propertyId, uint8_t* data, uint8_t length);
    virtual void functionPropertyExtStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                    uint8_t propertyId, uint8_t* data, uint8_t length);
    virtual void propertyValueReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                  uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool status);
    virtual void propertyValueReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                  uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    virtual void propertyValueWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                                uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool status);
    virtual void propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                              uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    virtual void propertyValueExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                              uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool confirmed);
    virtual void propertyDescriptionReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                     uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool status);
    virtual void propertyExtDescriptionReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                     uint16_t objectIndex, uint8_t propertyId, uint16_t propertyIndex, bool status);
    virtual void propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                   uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex);
    virtual void propertyExtDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                   uint16_t objectType, uint16_t objectInstance, uint16_t propertyId, uint8_t descriptionType, uint16_t propertyIndex);
    virtual void propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                 uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
                                                 uint16_t maxNumberOfElements, uint8_t access);
    virtual void propertyDescriptionReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
                                                        uint16_t maxNumberOfElements, uint8_t access, bool status);
    virtual void propertyDescriptionReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
                                                        uint16_t maxNumberOfElements, uint8_t access);
    virtual void memoryReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                        uint16_t memoryAddress, bool status);
    virtual void memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress);
    virtual void memoryReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint16_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint16_t memoryAddress, uint8_t* data);
    virtual void memoryWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                         uint16_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                       uint16_t memoryAddress, uint8_t* data);
    virtual void memoryRouterWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                       uint16_t memoryAddress, uint8_t* data);
    virtual void memoryRouterReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint16_t memoryAddress, uint8_t* data);
    virtual void memoryRoutingTableReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress);
    virtual void memoryRoutingTableReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t *data);
    virtual void memoryRoutingTableWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint16_t memoryAddress, uint8_t *data);
    virtual void memoryExtReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                        uint32_t memoryAddress, bool status);
    virtual void memoryExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress);
    virtual void memoryExtReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint32_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryExtReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint32_t memoryAddress, uint8_t* data);
    virtual void memoryExtWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                         uint32_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                       uint32_t memoryAddress, uint8_t* data);
    virtual void memoryExtWriteResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint32_t memoryAddress, uint8_t* data, bool status);
    virtual void memoryExtWriteAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint32_t memoryAddress, uint8_t* data);
    virtual void userMemoryReadLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                            uint32_t memoryAddress, bool status);
    virtual void userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                          uint32_t memoryAddress);
    virtual void userMemoryReadResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                               uint32_t memoryAddress, uint8_t* memoryData, bool status);
    virtual void userMemoryReadAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                               uint32_t memoryAddress, uint8_t* memoryData);
    virtual void userMemoryWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                             uint32_t memoryAddress, uint8_t* memoryData, bool status);
    virtual void userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                           uint32_t memoryAddress, uint8_t* memoryData);
    virtual void userManufacturerInfoLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, bool status);
    virtual void userManufacturerInfoIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl);
    virtual void userManufacturerInfoResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                     uint8_t* info, bool status);
    virtual void userManufacturerInfoAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                                     uint8_t* info);
    virtual void authorizeLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint32_t key, bool status);
    virtual void authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint32_t key);
    virtual void authorizeResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level,
                                          bool status);
    virtual void authorizeAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level);
    virtual void keyWriteLocalConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level,
                                      uint32_t key, bool status);
    virtual void keyWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level,
                                    uint32_t key);
    virtual void keyWriteResponseConfirm(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level,
                                         bool status);
    virtual void keyWriteAppLayerConfirm(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t level);
    virtual void connectConfirm(uint16_t destination);
    virtual void systemNetworkParameterReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                      uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength);

    virtual void domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                          const uint8_t* knxSerialNumber);

    virtual void domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber);

    virtual void systemNetworkParameterReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                        uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength, bool status);

    virtual void domainAddressSerialNumberWriteLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                            const uint8_t* knxSerialNumber, bool status);

    virtual void domainAddressSerialNumberReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber, bool status);

    virtual void propertyValueRead(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                   uint8_t& numberOfElements, uint16_t startIndex,
                                   uint8_t** data, uint32_t& length);

    virtual void propertyValueWrite(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                                    uint8_t& numberOfElements, uint16_t startIndex,
                                    uint8_t* data, uint32_t length);
    virtual void beforeRestartCallback(BeforeRestartCallback func);
    virtual BeforeRestartCallback beforeRestartCallback();
    virtual void functionPropertyCallback(FunctionPropertyCallback func);
    virtual FunctionPropertyCallback functionPropertyCallback();
    virtual void functionPropertyStateCallback(FunctionPropertyCallback func);
    virtual FunctionPropertyCallback functionPropertyStateCallback();
};
