#pragma once

#include "ip_host_protocol_address_information.h"
#include "knx_ip_frame.h"

namespace Knx
{
    class KnxIpSearchRequest : public KnxIpFrame
    {
        public:
            KnxIpSearchRequest(uint8_t* data, uint16_t length);
            IpHostProtocolAddressInformation& hpai();

        private:
            IpHostProtocolAddressInformation _hpai;
    };
} // namespace Knx