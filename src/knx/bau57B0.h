#pragma once

#include "config.h"
#ifdef USE_IP
#include "bau_systemB.h"
#include "ip_parameter_object.h"
#include "ip_data_link_layer.h"

class Bau57B0 : public BauSystemB
{
  public:
    Bau57B0(Platform& platform);

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);
    uint8_t* descriptor();
    DataLinkLayer& dataLinkLayer();

  private:
    IpParameterObject _ipParameters;
    IpDataLinkLayer _dlLayer;
    uint8_t _descriptor[2] = {0x57, 0xb0};
    const uint32_t _ifObjs[7] = { 6, // length
                                  OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG, OT_IP_PARAMETER};
};
#endif