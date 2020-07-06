#pragma once

#include "config.h"
#include "bau.h"
#include "device_object.h"
#include "address_table_object.h"
#include "association_table_object.h"
#include "group_object_table_object.h"
#include "security_interface_object.h"
#include "application_program_object.h"
#include "application_layer.h"
#include "secure_application_layer.h"
#include "transport_layer.h"
#include "network_layer.h"
#include "data_link_layer.h"
#include "platform.h"
#include "memory.h"

class BauSystemB : protected BusAccessUnit
{
  public:
    BauSystemB(Platform& platform);
    virtual void loop();
    DeviceObject& deviceObject();
    GroupObjectTableObject& groupObjectTable();
    ApplicationProgramObject& parameters();
    Memory& memory();
    bool configured();
    bool enabled();
    void enabled(bool value);
    void readMemory();
    void writeMemory();
    void addSaveRestore(SaveRestore* obj);
    bool restartRequest(uint16_t asap, const SecurityControl secCtrl);
    uint8_t checkmasterResetValidity(EraseCode eraseCode, uint8_t channel);

    void propertyValueRead(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                           uint8_t& numberOfElements, uint16_t startIndex, 
                           uint8_t **data, uint32_t &length) override;
    void propertyValueWrite(ObjectType objectType, uint8_t objectInstance, uint8_t propertyId,
                            uint8_t& numberOfElements, uint16_t startIndex,
                            uint8_t* data, uint32_t length) override;

  protected:
    virtual DataLinkLayer& dataLinkLayer() = 0;
    void memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                               uint16_t memoryAddress, uint8_t* data) override;
    void memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                              uint16_t memoryAddress) override;
    void memoryExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                  uint32_t memoryAddress, uint8_t* data) override;
    void memoryExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                 uint32_t memoryAddress) override;
    void deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t descriptorType) override;
    void restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, RestartType restartType, EraseCode eraseCode, uint8_t channel) override;
    void authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint32_t key) override;
    void userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number, uint32_t memoryAddress) override;
    void userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                   uint32_t memoryAddress, uint8_t* memoryData) override;
    void propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                           uint8_t propertyId, uint8_t propertyIndex) override;
    void propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                      uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length) override;
    void propertyValueExtWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                         uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length, bool confirmed);
    void propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                     uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex) override;
    void propertyValueExtReadIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex) override;
    void functionPropertyCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                           uint8_t propertyId, uint8_t* data, uint8_t length) override;
    void functionPropertyStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t objectIndex,
                                         uint8_t propertyId, uint8_t* data, uint8_t length) override;
    void functionPropertyExtCommandIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                      uint8_t propertyId, uint8_t* data, uint8_t length) override;
    void functionPropertyExtStateIndication(Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, ObjectType objectType, uint8_t objectInstance,
                                                    uint8_t propertyId, uint8_t* data, uint8_t length) override;
    void individualAddressReadIndication(HopCountType hopType, const SecurityControl &secCtrl) override;
    void individualAddressWriteIndication(HopCountType hopType, const SecurityControl &secCtrl, uint16_t newaddress) override;
    void individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t newIndividualAddress,
                                                      uint8_t* knxSerialNumber) override;
    void individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* knxSerialNumber) override;
    void groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                     uint8_t* data, uint8_t dataLength, bool status) override;
    void groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, bool status) override;
    void groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl) override;
    void groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                       uint8_t* data, uint8_t dataLength) override;
    void groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                   uint8_t* data, uint8_t dataLength) override;
    void systemNetworkParameterReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                              uint16_t propertyId, uint8_t* testInfo, uint16_t testinfoLength) override;
    void systemNetworkParameterReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint16_t objectType,
                                                uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength, bool status) override;
    void connectConfirm(uint16_t tsap) override;

    virtual InterfaceObject* getInterfaceObject(uint8_t idx) = 0;
    virtual InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance) = 0;
    void sendNextGroupTelegram();
    void updateGroupObject(GroupObject& go, uint8_t* data, uint8_t length);
    void nextRestartState();

    virtual void doMasterReset(EraseCode eraseCode, uint8_t channel);

    enum RestartState
    {
        Idle,
        Connecting,
        Connected,
        Restarted
    };

    DeviceObject _deviceObj;
    Memory _memory;
    AddressTableObject _addrTable;
    AssociationTableObject _assocTable;
    GroupObjectTableObject _groupObjTable;
    ApplicationProgramObject _appProgram;
    Platform& _platform;
#ifdef USE_DATASECURE
    SecureApplicationLayer _appLayer;
    SecurityInterfaceObject _secIfObj;
#else
    ApplicationLayer _appLayer;
#endif
    TransportLayer _transLayer;
    NetworkLayer _netLayer;
    bool _configured = true;
    RestartState _restartState = Idle;
    SecurityControl _restartSecurity;
    uint32_t _restartDelay = 0;
};
