#pragma once

#include "config.h"
#if defined(USE_IP) && defined (USE_TP)
#include "bau_systemB_coupler.h"
#include "ip_parameter_object.h"
#include "ip_data_link_layer.h"
#include "tpuart_data_link_layer.h"
#include "cemi_server_object.h"

class Bau091A : public BauSystemBCoupler, public ITpUartCallBacks
{
  public:
    Bau091A(Platform& platform);
    virtual void loop() override;
    virtual bool enabled() override;
    virtual void enabled(bool value) override;

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);

    // For TP1 only
    virtual bool isAckRequired(uint16_t address, bool isGrpAddr) override;

    virtual void doMasterReset(EraseCode eraseCode, uint8_t channel) override;
  private:
    IpParameterObject _ipParameters;
    IpDataLinkLayer _dlLayerPrimary;
    TpUartDataLinkLayer _dlLayerSecondary;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif
};
#endif
