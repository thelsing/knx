#pragma once

#include "config.h"
#if MASK_VERSION == 0x091A

#include "bau_systemB_coupler.h"
#include "router_object.h"
#include "ip_parameter_object.h"
#include "ip_data_link_layer.h"
#include "tpuart_data_link_layer.h"
#include "cemi_server_object.h"

class Bau091A : public BauSystemBCoupler, public ITpUartCallBacks, public DataLinkLayerCallbacks
{
  public:
    Bau091A(Platform& platform);
    void loop() override;
    bool enabled() override;
    void enabled(bool value) override;

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);

    // For TP1 only
    bool isAckRequired(uint16_t address, bool isGrpAddr) override;

    void doMasterReset(EraseCode eraseCode, uint8_t channel) override;
  private:
    RouterObject _routerObj;
    IpParameterObject _ipParameters;
    IpDataLinkLayer _dlLayerPrimary;
    TpUartDataLinkLayer _dlLayerSecondary;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif
};
#endif
