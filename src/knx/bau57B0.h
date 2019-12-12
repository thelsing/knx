#pragma once

#include "config.h"
#ifdef USE_IP
#include "bau_systemB.h"
#include "ip_parameter_object.h"
#include "ip_data_link_layer.h"
#include "cemi_server_object.h"

class Bau57B0 : public BauSystemB
{
  public:
    Bau57B0(Platform& platform);

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);
    DataLinkLayer& dataLinkLayer();

  private:
    IpParameterObject _ipParameters;
    IpDataLinkLayer _dlLayer;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;

    const uint32_t _ifObjs[8] = { 8, // length
                                 OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG, OT_IP_PARAMETER, OT_CEMI_SERVER};
#else
    const uint32_t _ifObjs[7] = {7, // length
                                 OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG, OT_IP_PARAMETER};
#endif
};
#endif