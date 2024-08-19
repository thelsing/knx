#pragma once

#include "router_object.h"
#include "network_layer_coupler.h"

#include "../config.h"
#include "../bau/bau_systemB.h"
#include "../interface_object/device_object.h"
#include "../interface_object/application_program_object.h"
#include "../data_secure/security_interface_object.h"
#include "../data_secure/secure_application_layer.h"
#include "../application_layer/application_layer.h"
#include "../transport_layer/transport_layer.h"
#include "../datalink_layer/data_link_layer.h"
#include "../platform/platform.h"
#include "../util/memory.h"

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
