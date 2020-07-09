#pragma once

#include "config.h"
#include "bau_systemB_device.h"
#include "tpuart_data_link_layer.h"
#include "cemi_server.h"
#include "cemi_server_object.h"

#ifdef USE_TP

class Bau07B0 : public BauSystemBDevice
{
  public:
    Bau07B0(Platform& platform);
    void loop();
    
  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);
    DataLinkLayer& dataLinkLayer();

  private:
    TpUartDataLinkLayer _dlLayer;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif                                  
};
#endif
