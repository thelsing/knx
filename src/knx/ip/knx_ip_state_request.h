#pragma once

#include "ip_host_protocol_address_information.h"
#include "knx_ip_cri.h"
#include "knx_ip_frame.h"

namespace Knx
{
    class KnxIpStateRequest : public KnxIpFrame
    {
        public:
            KnxIpStateRequest(uint8_t* data, uint16_t length);
            IpHostProtocolAddressInformation& hpaiCtrl();
            uint8_t channelId();

        private:
            IpHostProtocolAddressInformation _hpaiCtrl;
    };
} // namespace Knx