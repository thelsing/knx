#pragma once

#include "bau_systemB.h"
#include "rf_medium_object.h"
#include "rf_physical_layer.h"
#include "rf_data_link_layer.h"

class Bau27B0 : public BauSystemB
{
  public:
    Bau27B0(Platform& platform);

  protected:
    InterfaceObject* getInterfaceObject(uint8_t idx);
    uint8_t* descriptor();
    DataLinkLayer& dataLinkLayer();

  private:
    RfDataLinkLayer _dlLayer;
    RfMediumObject _rfMediumObj;

    uint8_t _descriptor[2] = {0x27, 0xB0};

    void domainAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, uint8_t* rfDoA,
                                                  uint8_t* knxSerialNumber);
    void domainAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, uint8_t* knxSerialNumber);
    void individualAddressSerialNumberWriteIndication(Priority priority, HopCountType hopType, uint16_t newIndividualAddress,
                                                      uint8_t* knxSerialNumber);
    void individualAddressSerialNumberReadIndication(Priority priority, HopCountType hopType, uint8_t* knxSerialNumber);
};