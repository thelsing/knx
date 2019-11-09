#pragma once

#include "interface_object.h"

class CemiServerObject: public InterfaceObject
{
public:
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count);
    uint8_t propertySize(PropertyID id);
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
    void readPropertyDescription(uint8_t propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access);
    ObjectType objectType() { return OT_CEMI_SERVER; }

protected:
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();
private:
    // cEMI additional info types supported by this cEMI server: only 0x02 (RF Control Octet and Serial Number or DoA)
    uint8_t _addInfoTypesTable[1] = { 0x02 }; 
    uint8_t _commMode = 0x00;

};