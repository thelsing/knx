#pragma once
#include "knx_ip_dib.h"

namespace Knx
{
    class KnxIpKnxAddressesDIB : public KnxIpDIB
    {
        public:
            KnxIpKnxAddressesDIB(uint8_t* data);
            uint16_t individualAddress();
            void individualAddress(uint16_t addr);
            void additional(uint16_t addr);

        private:
            uint8_t* currentPos = 0;
    };
} // namespace Knx