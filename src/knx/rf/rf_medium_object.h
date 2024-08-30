#pragma once

#include "../interface_object/interface_object.h"

namespace Knx
{
    class RfMediumObject: public InterfaceObject
    {
        public:
            RfMediumObject();
            const uint8_t* rfDomainAddress();
            void rfDomainAddress(const uint8_t* value);
            const char* name() override
            {
                return "RfMediumObject";
            }
        private:
            uint8_t _rfDiagSourceAddressFilterTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
            uint8_t _rfDiagLinkBudgetTable[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
    };
}