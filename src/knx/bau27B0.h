#pragma once

#include "config.h"
#if MASK_VERSION == 0x27B0

#include "bau_systemB_device.h"
#include "rf_medium_object.h"
#if defined(DeviceFamily_CC13X0)
  #include "rf_physical_layer_cc1310.h"
#else
  #include "rf_physical_layer_cc1101.h"
#endif
#include "rf_data_link_layer.h"
#include "cemi_server.h"
#include "cemi_server_object.h"

class Bau27B0 : public BauSystemBDevice
{
  public:
    Bau27B0(Platform& platform);
    virtual void loop() override;
    virtual bool enabled() override;
    virtual void enabled(bool value) override;

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);

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
