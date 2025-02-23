#pragma once
#include "knx_ip_dib.h"
#include "service_families.h"

namespace Knx
{
    class KnxIpTunnelingInfoDIB : public KnxIpDIB
    {
        public:
            KnxIpTunnelingInfoDIB(uint8_t* data);
            uint16_t apduLength();
            void apduLength(uint16_t addr);
            void tunnelingSlot(uint16_t addr, uint16_t state);

        private:
            uint8_t* currentPos = 0;
    };
} // namespace Knx