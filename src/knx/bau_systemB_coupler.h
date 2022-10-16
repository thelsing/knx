#pragma once

#include "config.h"
#include "bau_systemB.h"
#include "device_object.h"
#include "security_interface_object.h"
#include "application_program_object.h"
#include "router_object.h"
#include "application_layer.h"
#include "secure_application_layer.h"
#include "transport_layer.h"
#include "network_layer_coupler.h"
#include "data_link_layer.h"
#include "platform.h"
#include "memory.h"

class BauSystemBCoupler : public BauSystemB
{
  public:
    BauSystemBCoupler(Platform& platform);
    void loop() override;
    bool configured() override;

  protected:
    ApplicationLayer& applicationLayer() override;

    void doMasterReset(EraseCode eraseCode, uint8_t channel) override;

    Platform& _platform;

#ifdef USE_DATASECURE
    SecureApplicationLayer _appLayer;
    SecurityInterfaceObject _secIfObj;
#else
    ApplicationLayer _appLayer;
#endif
    TransportLayer _transLayer;
    NetworkLayerCoupler _netLayer;
    bool _configured = true;
};
