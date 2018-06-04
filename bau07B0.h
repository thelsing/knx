#pragma once

#include "bau.h"
#include "device_object.h"
#include "address_table_object.h"
#include "association_table_object.h"
#include "group_object_table_object.h"
#include "application_program_object.h"
#include "application_layer.h"
#include "transport_layer.h"
#include "network_layer.h"
#include "tpuart_data_link_layer.h"
#include "platform.h"
#include "memory.h"

class Bau07B0: protected BusAccessUnit
{
    using BusAccessUnit::memoryReadIndication;
public:
    Bau07B0(Platform& platform);
    void loop();
    DeviceObject& deviceObject();
    GroupObjectTableObject& groupObjectTable();
    ApplicationProgramObject& parameters();
    bool configured();
    bool enabled();
    void enabled(bool value);
    void readMemory();
protected:
    void memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress, uint8_t* data) override;
    void memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
        uint16_t memoryAddress) override;
    void deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t descriptorType);
    void restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap);
    void authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, uint32_t key);
    void userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress);
    void userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, 
        uint32_t memoryAddress, uint8_t* memoryData);
    void propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex, 
        uint8_t propertyId, uint8_t propertyIndex);
    void propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex, 
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex, 
        uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    void individualAddressReadIndication(HopCountType hopType);
    void individualAddressWriteIndication(HopCountType hopType, uint16_t newaddress);
    void groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType,
        uint8_t* data, uint8_t dataLength, bool status);
    void groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, bool status);
    void groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType);
    void groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType,
        uint8_t* data, uint8_t dataLength);
    void groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType,
        uint8_t* data, uint8_t dataLength);
    
    InterfaceObject* getInterfaceObject(uint8_t idx);
    void sendNextGroupTelegram();
    void updateGroupObject(GroupObject& go, uint8_t* data, uint8_t length);
private:
    DeviceObject _deviceObj;
    // pointer to first private variable as reference to memory read/write commands
    uint8_t* _memoryReference;
    Memory _memory;
    AddressTableObject _addrTable;
    AssociationTableObject _assocTable;
    GroupObjectTableObject _groupObjTable;
    ApplicationProgramObject _appProgram;
    Platform& _platform;
    ApplicationLayer _appLayer;
    TransportLayer _transLayer;
    NetworkLayer _netLayer;
    TpUartDataLinkLayer _dlLayer;

};