#pragma once

#include "knx_ip_frame.h"

namespace Knx
{
    class KnxIpDisconnectResponse : public KnxIpFrame
    {
        public:
            KnxIpDisconnectResponse(uint8_t channel, uint8_t status);

        private:
    };
} // namespace Knx