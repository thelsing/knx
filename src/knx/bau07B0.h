#pragma once

#include "bau_systemB.h"
#include "tpuart_data_link_layer.h"

class Bau07B0 : public BauSystemB
{
  public:
    Bau07B0(Platform& platform);

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    uint8_t* descriptor();
    DataLinkLayer& dataLinkLayer();

  private:
    TpUartDataLinkLayer _dlLayer;
    uint8_t _descriptor[2] = {0x07, 0xb0};
    const uint32_t _ifObjs[6] = { 5, // length
                                  OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG};
};