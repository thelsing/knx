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
#ifdef USE_USBIF
#include "usb_data_link_layer.h"
#include "cemi_server.h"
#include "cemi_server_object.h"
#endif

class BauSystemB : protected BusAccessUnit
{
  public:
    BauSystemB(Platform& platform);
    virtual void loop();
    DeviceObject& deviceObject();
    GroupObjectTableObject& groupObjectTable();
    ApplicationProgramObject& parameters();
    bool configured();
    bool enabled();
    void enabled(bool value);
    void readMemory();
    void writeMemory();
    void addSaveRestore(SaveRestore* obj);
    bool restartRequest(uint16_t asap);

  protected:
    virtual DataLinkLayer& dataLinkLayer() = 0;
    virtual uint8_t* descriptor() = 0;
    void memoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                               uint16_t memoryAddress, uint8_t* data) override;
    void memoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                              uint16_t memoryAddress) override;
    void deviceDescriptorReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t descriptorType) override;
    void restartRequestIndication(Priority priority, HopCountType hopType, uint16_t asap) override;
    void authorizeIndication(Priority priority, HopCountType hopType, uint16_t asap, uint32_t key) override;
    void userMemoryReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number, uint32_t memoryAddress) override;
    void userMemoryWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                                   uint32_t memoryAddress, uint8_t* memoryData) override;
    void propertyDescriptionReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
                                           uint8_t propertyId, uint8_t propertyIndex) override;
    void propertyValueWriteIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
                                      uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length) override;
    void propertyValueReadIndication(Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
                                     uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex) override;
    void individualAddressReadIndication(HopCountType hopType) override;
    void individualAddressWriteIndication(HopCountType hopType, uint16_t newaddress) override;
    void groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType,
                                     uint8_t* data, uint8_t dataLength, bool status) override;
    void groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, bool status) override;
    void groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType) override;
    void groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType,
                                       uint8_t* data, uint8_t dataLength) override;
    void groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType,
                                   uint8_t* data, uint8_t dataLength) override;
    void systemNetworkParameterReadIndication(Priority priority, HopCountType hopType, uint16_t objectType,
                                              uint16_t propertyId, uint8_t* testInfo, uint16_t testinfoLength);
    void connectConfirm(uint16_t tsap) override;

    virtual InterfaceObject* getInterfaceObject(uint8_t idx) = 0;
    void sendNextGroupTelegram();
    void updateGroupObject(GroupObject& go, uint8_t* data, uint8_t length);
    void nextRestartState();

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
    ApplicationLayer _appLayer;
    TransportLayer _transLayer;
    NetworkLayer _netLayer;
    bool _configured = true;
    RestartState _restartState = Idle;
    uint32_t _restartDelay = 0;
#ifdef USE_USBIF
    UsbDataLinkLayer _dlLayerUsb;
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif
};