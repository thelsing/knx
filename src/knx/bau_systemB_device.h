#pragma once

#include "config.h"
#include "bau_systemB.h"
#include "device_object.h"
#include "address_table_object.h"
#include "association_table_object.h"
#include "group_object_table_object.h"
#include "security_interface_object.h"
#include "application_program_object.h"
#include "application_layer.h"
#include "secure_application_layer.h"
#include "transport_layer.h"
#include "network_layer_device.h"
#include "data_link_layer.h"
#include "platform.h"
#include "memory.h"

class BauSystemBDevice : public BauSystemB
{
  public:
    BauSystemBDevice(Platform& platform);
    void loop() override;
    bool configured() override;
    GroupObjectTableObject& groupObjectTable();

  protected:
    ApplicationLayer& applicationLayer() override;

    void groupValueWriteLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                     uint8_t* data, uint8_t dataLength, bool status) override;
    void groupValueReadLocalConfirm(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, bool status) override;
    void groupValueReadIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl) override;
    void groupValueReadAppLayerConfirm(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                       uint8_t* data, uint8_t dataLength) override;
    void groupValueWriteIndication(uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl,
                                   uint8_t* data, uint8_t dataLength) override;

    void sendNextGroupTelegram();
    void updateGroupObject(GroupObject& go, uint8_t* data, uint8_t length);

    void doMasterReset(EraseCode eraseCode, uint8_t channel) override;

    AddressTableObject _addrTable;
    AssociationTableObject _assocTable;
    GroupObjectTableObject _groupObjTable;
#ifdef USE_DATASECURE
    SecureApplicationLayer _appLayer;
    SecurityInterfaceObject _secIfObj;
#else
    ApplicationLayer _appLayer;
#endif
    TransportLayer _transLayer;
    NetworkLayerDevice _netLayer;

    bool _configured = true;
};
