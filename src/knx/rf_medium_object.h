#pragma once

#include "config.h"
#ifdef USE_RF

#include "interface_object.h"

class RfMediumObject: public InterfaceObject
{
public:
    RfMediumObject();
    const uint8_t* rfDomainAddress();
    void rfDomainAddress(const uint8_t* value);

private:
    uint8_t _rfDiagSourceAddressFilterTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
    uint8_t _rfDiagLinkBudgetTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
};
#endif
