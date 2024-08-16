#include "../config.h"
#ifdef USE_IP
#include "knx_ip_search_request.h"

KnxIpSearchRequest::KnxIpSearchRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpai(data + LEN_KNXIP_HEADER)
{
}


IpHostProtocolAddressInformation& KnxIpSearchRequest::hpai()
{
    return _hpai;
}
#endif