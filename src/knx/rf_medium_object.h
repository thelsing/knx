#pragma once

#include "interface_object.h"

class RfMediumObject: public InterfaceObject
{
public:
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    void writeProperty(PropertyID id, uint32_t start, uint8_t* data, uint32_t& count);
    uint8_t propertySize(PropertyID id);
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
    void readPropertyDescription(uint8_t propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access);
    ObjectType objectType() { return OT_RF_MEDIUM; }

    uint8_t* rfDomainAddress();
    void rfDomainAddress(uint8_t* value);

protected:
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();
private:
    uint8_t _rfDomainAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // see KNX RF S-Mode AN160 p.11
    uint8_t _rfDiagSourceAddressFilterTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
    uint8_t _rfDiagLinkBudgetTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};


};