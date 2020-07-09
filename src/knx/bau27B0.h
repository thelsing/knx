#pragma once

#include "config.h"
#ifdef USE_RF
#include "bau_systemB_device.h"
#include "rf_medium_object.h"
#include "rf_physical_layer.h"
#include "rf_data_link_layer.h"
#include "cemi_server.h"
#include "cemi_server_object.h"

class Bau27B0 : public BauSystemBDevice
{
  public:
    Bau27B0(Platform& platform);
    void loop();

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);
    DataLinkLayer& dataLinkLayer();

    virtual void doMasterReset(EraseCode eraseCode, uint8_t channel) override;
  private:
    RfDataLinkLayer _dlLayer;
    RfMediumObject _rfMediumObj;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif

    void domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                  const uint8_t* knxSerialNumber) override;
    void domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber) override;
    void individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* knxSerialNumber) override;
    void domainAddressSerialNumberWriteLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* rfDoA,
                                                    const uint8_t* knxSerialNumber, bool status) override;
    void domainAddressSerialNumberReadLocalConfirm(Priority priority, HopCountType hopType, const SecurityControl &secCtrl, const uint8_t* knxSerialNumber, bool status) override;
};
#endif
