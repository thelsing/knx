#include "knx_ip_state_request.h"
#ifdef USE_IP
KnxIpStateRequest::KnxIpStateRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpaiCtrl(data + LEN_KNXIP_HEADER + 1 /*ChannelId*/ + 1 /*Reserved*/)
{
}

IpHostProtocolAddressInformation& KnxIpStateRequest::hpaiCtrl()
{
    return _hpaiCtrl;
}
uint8_t KnxIpStateRequest::channelId()
{
    return _data[LEN_KNXIP_HEADER];
}
#endif