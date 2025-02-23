#pragma once

#include "application_layer.h"
#include "application_program_object.h"
#include "bau_systemB.h"
#include "config.h"
#include "data_link_layer.h"
#include "device_object.h"
#include "memory.h"
#include "network_layer_coupler.h"
#include "platform.h"
#include "router_object.h"
#include "secure_application_layer.h"
#include "security_interface_object.h"
#include "transport_layer.h"

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
