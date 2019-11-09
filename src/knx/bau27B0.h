#pragma once

#include "bau_systemB.h"
#include "rf_medium_object.h"
#include "rf_physical_layer.h"
#include "rf_data_link_layer.h"
#ifdef USE_CEMI_SERVER
#include "cemi_server.h"
#include "cemi_server_object.h"
#endif

class Bau27B0 : public BauSystemB
{
  public:
    Bau27B0(Platform& platform);
    void enabled(bool value);
    void loop();

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    InterfaceObject* getInterfaceObject(ObjectType objectType, uint8_t objectInstance);
    uint8_t* descriptor();
    DataLinkLayer& dataLinkLayer();

  private:
    RfDataLinkLayer _dlLayer;
    RfMediumObject _rfMediumObj;
#ifdef USE_CEMI_SERVER
    CemiServer _cemiServer;
    CemiServerObject _cemiServerObject;
#endif

    uint8_t _descriptor[2] = {0x27, 0xB0};
#ifdef USE_CEMI_SERVER
    const uint32_t _ifObjs[8] = { 7, // length
                                  OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG, OT_RF_MEDIUM, OT_CEMI_SERVER};
#else    
    const uint32_t _ifObjs[7] = { 6, // length
                                  OT_DEVICE, OT_ADDR_TABLE, OT_ASSOC_TABLE, OT_GRP_OBJ_TABLE, OT_APPLICATION_PROG, OT_RF_MEDIUM};
#endif                                  

    void domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, uint8_t* rfDoA,
                                                  uint8_t* knxSerialNumber);
    void domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, uint8_t* knxSerialNumber);
    void individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, uint16_t newIndividualAddress,
                                                      uint8_t* knxSerialNumber);
    void individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, uint8_t* knxSerialNumber);
};