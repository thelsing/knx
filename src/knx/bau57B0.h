#pragma once

#include "config.h"
#ifdef USE_IP
#include "bau_systemB_device.h"
#include "ip_parameter_object.h"
#include "ip_data_link_layer.h"
#include "cemi_server_object.h"

class Bau57B0 : public BauSystemBDevice
{
  public:
    Bau57B0(Platform& platform);
    virtual void loop() override;
    virtual bool enabled() override;
    virtual void enabled(bool value) override;

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);

    virtual void doMasterReset(EraseCode eraseCode, uint8_t channel) override;
  private:
    IpParameterObject _ipParameters;
    IpDataLinkLayer _dlLayer;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif
};
#endif
