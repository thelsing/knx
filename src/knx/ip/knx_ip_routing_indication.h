#pragma once

#include "../datalink_layer/cemi_frame.h"
#include "knx_ip_frame.h"

namespace Knx
{
    class KnxIpRoutingIndication : public KnxIpFrame
    {
        public:
            KnxIpRoutingIndication(uint8_t* data, uint16_t length);
            KnxIpRoutingIndication(CemiFrame frame);
            CemiFrame& frame();

        private:
            CemiFrame _frame;
    };
} // namespace Knx