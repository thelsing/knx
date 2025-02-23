#pragma once

#include "ip_host_protocol_address_information.h"
#include "knx_ip_cri.h"
#include "knx_ip_frame.h"
#ifdef USE_IP
class KnxIpConnectRequest : public KnxIpFrame
{
    public:
        KnxIpConnectRequest(uint8_t* data, uint16_t length);
        IpHostProtocolAddressInformation& hpaiCtrl();
        IpHostProtocolAddressInformation& hpaiData();
        KnxIpCRI& cri();

    private:
        IpHostProtocolAddressInformation _hpaiCtrl;
        IpHostProtocolAddressInformation _hpaiData;
        KnxIpCRI _cri;
};
#endif