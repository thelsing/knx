#pragma once

#include "knx_ip_frame.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP

class KnxIpSearchResponse : public KnxIpFrame
{
    IpHostProtocolAddressInformation& controlEndpoint();
    
};

#endif