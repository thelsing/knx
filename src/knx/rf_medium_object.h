#pragma once

#include "config.h"
#ifdef USE_RF
#include "interface_object.h"

class RfMediumObject: public InterfaceObject
{
public:
    void readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data) override;
    void writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count) override;
    uint8_t propertySize(PropertyID id) override;
    uint8_t* save(uint8_t* buffer) override;
    const uint8_t* restore(const uint8_t* buffer) override;
    uint16_t saveSize() override;

    uint8_t* rfDomainAddress();
    void rfDomainAddress(uint8_t* value);

protected:
    uint8_t propertyDescriptionCount() override;
    PropertyDescription* propertyDescriptions() override;
private:
    uint8_t _rfDomainAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // see KNX RF S-Mode AN160 p.11
    uint8_t _rfDiagSourceAddressFilterTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
    uint8_t _rfDiagLinkBudgetTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
};
#endif