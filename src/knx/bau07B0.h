#pragma once

#include "bau_systemB.h"
#include "tpuart_data_link_layer.h"
#include "cemi_server.h"
#include "cemi_server_object.h"

class Bau07B0 : public BauSystemB
{
  public:
    Bau07B0(Platform& platform);
    void loop();
    
  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);
    uint8_t* descriptor();
    DataLinkLayer& dataLinkLayer();

  private:
    TpUartDataLinkLayer _dlLayer;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif

    uint8_t _descriptor[2] = {0x07, 0xb0};
#ifdef USE_CEMI_SERVER
    const uint32_t _ifObjs[7] = { 6, // length
                                  OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG, OT_CEMI_SERVER};
#else    
    const uint32_t _ifObjs[6] = { 5, // length
                                  OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG};
#endif                                  
};