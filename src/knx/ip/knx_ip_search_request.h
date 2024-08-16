#pragma once

#include "knx_ip_frame.h"
#include "ip_host_protocol_address_information.h"

class KnxIpSearchRequest : public KnxIpFrame
{
    public:
        KnxIpSearchRequest(uint8_t* data, uint16_t length);
        IpHostProtocolAddressInformation& hpai();
    private:
        IpHostProtocolAddressInformation _hpai;
};
