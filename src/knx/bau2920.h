#pragma once

#include "config.h"
#if MASK_VERSION == 0x2920

#include "bau_systemB_coupler.h"
#include "tpuart_data_link_layer.h"
#if defined(DeviceFamily_CC13X0)
  #include "rf_physical_layer_cc1310.h"
#else
  #include "rf_physical_layer_cc1101.h"
#endif  
#include "rf_data_link_layer.h"
#include "rf_medium_object.h"
#include "cemi_server_object.h"

class Bau2920 : public BauSystemBCoupler
{
  public:
    Bau2920(Platform& platform);
    virtual void loop() override;
    virtual bool enabled() override;
    virtual void enabled(bool value) override;

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);

    virtual void doMasterReset(EraseCode eraseCode, uint8_t channel) override;
  private:
    RouterObject _rtObjPrimary;
    RouterObject _rtObjSecondary;
    RfMediumObject _rfMediumObject;
    TpUartDataLinkLayer _dlLayerPrimary;
    RfDataLinkLayer _dlLayerSecondary;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif
};
#endif
