#include "knx_ip_disconnect_request.h"
#ifdef USE_IP
KnxIpDisconnectRequest::KnxIpDisconnectRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpaiCtrl(data + LEN_KNXIP_HEADER + 1 /*ChannelId*/ + 1 /*Reserved*/)
{
}

KnxIpDisconnectRequest::KnxIpDisconnectRequest()
    : KnxIpFrame(1 /*ChannelId*/ + 1 /*Reserved*/ + LEN_KNXIP_HEADER + LEN_IPHPAI), _hpaiCtrl(_data + 1 /*ChannelId*/ + 1 /*Reserved*/ + LEN_KNXIP_HEADER)
{
    serviceTypeIdentifier(DisconnectRequest);
}

IpHostProtocolAddressInformation& KnxIpDisconnectRequest::hpaiCtrl()
{
    return _hpaiCtrl;
}
uint8_t KnxIpDisconnectRequest::channelId()
{
    return _data[LEN_KNXIP_HEADER];
}
void KnxIpDisconnectRequest::channelId(uint8_t channelId)
{
    _data[LEN_KNXIP_HEADER] = channelId;
}
#endif