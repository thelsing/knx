#pragma once

#include "bau_systemB.h"
#include "ip_parameter_object.h"
#include "ip_data_link_layer.h"

class Bau57B0 : public BauSystemB
{
  public:
    Bau57B0(Platform& platform);

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    uint8_t* descriptor();
    DataLinkLayer& dataLinkLayer();

  private:
    IpParameterObject _ipParameters;
    IpDataLinkLayer _dlLayer;
    uint8_t _descriptor[2] = {0x57, 0xb0};
};