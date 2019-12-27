#include "knx_ip_search_request.h"
#ifdef USE_IP
KnxIpSearchRequest::KnxIpSearchRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpai(data + KNXIP_HEADER_LEN)
{
}


IpHostProtocolAddressInformation& KnxIpSearchRequest::hpai()
{
    return _hpai;
}
#endif