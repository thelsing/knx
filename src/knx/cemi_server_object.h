#pragma once

#include "config.h"
#ifdef USE_CEMI_SERVER

#include "interface_object.h"

class CemiServerObject: public InterfaceObject
{
public:
    void readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data) override;
    void writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count) override;
    uint8_t propertySize(PropertyID id) override;

protected:
  uint8_t propertyDescriptionCount() override;
  PropertyDescription* propertyDescriptions() override;
private:
    // cEMI additional info types supported by this cEMI server: only 0x02 (RF Control Octet and Serial Number or DoA)
    uint8_t _addInfoTypesTable[1] = { 0x02 }; 
    uint8_t _commMode = 0x00;

};
#endif