#pragma once

#include "ip_host_protocol_address_information.h"
#include "knx_ip_cri.h"
#include "knx_ip_frame.h"

namespace Knx
{
    class KnxIpDescriptionRequest : public KnxIpFrame
    {
        public:
            KnxIpDescriptionRequest(uint8_t* data, uint16_t length);
            IpHostProtocolAddressInformation& hpaiCtrl();

        private:
            IpHostProtocolAddressInformation _hpaiCtrl;
    };
} // namespace Knx