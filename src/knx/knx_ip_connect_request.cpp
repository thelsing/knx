#include "knx_ip_connect_request.h"
#ifdef USE_IP
KnxIpConnectRequest::KnxIpConnectRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpaiCtrl(data + LEN_KNXIP_HEADER), _hpaiData(data + LEN_KNXIP_HEADER + LEN_IPHPAI), _cri(data + LEN_KNXIP_HEADER + 2*LEN_IPHPAI)
{
}


IpHostProtocolAddressInformation& KnxIpConnectRequest::hpaiCtrl()
{
    return _hpaiCtrl;
}
IpHostProtocolAddressInformation& KnxIpConnectRequest::hpaiData()
{
    return _hpaiData;
}
KnxIpCRI& KnxIpConnectRequest::cri()
{
    return _cri;
}
#endif