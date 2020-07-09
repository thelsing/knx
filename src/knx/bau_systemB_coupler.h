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
#include "network_layer.h"
#include "data_link_layer.h"
#include "platform.h"
#include "memory.h"

class BauSystemBCoupler : public BauSystemB
{
  public:
    BauSystemBCoupler(Platform& platform);
    virtual void loop();
    bool configured();

  protected:
    virtual DataLinkLayer& dataLinkLayer() = 0;
    virtual ApplicationLayer& applicationLayer() override;

    virtual void doMasterReset(EraseCode eraseCode, uint8_t channel);

    enum RestartState
    {
        Idle,
        Connecting,
        Connected,
        Restarted
    };

    Platform& _platform;
#ifdef USE_DATASECURE
    SecureApplicationLayer _appLayer;
    SecurityInterfaceObject _secIfObj;
#else
    ApplicationLayer _appLayer;
#endif
    TransportLayer _transLayer;
    NetworkLayer _netLayer;
    bool _configured = true;
    RestartState _restartState = Idle;
    SecurityControl _restartSecurity;
    uint32_t _restartDelay = 0;
};
