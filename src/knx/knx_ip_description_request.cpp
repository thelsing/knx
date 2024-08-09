#include "knx_ip_description_request.h"
#ifdef USE_IP
KnxIpDescriptionRequest::KnxIpDescriptionRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpaiCtrl(data + LEN_KNXIP_HEADER)
{
}


IpHostProtocolAddressInformation& KnxIpDescriptionRequest::hpaiCtrl()
{
    return _hpaiCtrl;
}
#endif