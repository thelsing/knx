#include "knx_ip_tunneling_ack.h"
#include <cstring>

#ifdef USE_IP
KnxIpTunnelingAck::KnxIpTunnelingAck(uint8_t* data, 
    uint16_t length) : KnxIpFrame(data, length), _ch(_data + LEN_KNXIP_HEADER)
{
}

KnxIpTunnelingAck::KnxIpTunnelingAck()
    : KnxIpFrame(LEN_KNXIP_HEADER + LEN_CH), _ch(_data + LEN_KNXIP_HEADER)
{
    serviceTypeIdentifier(TunnelingAck);
}

KnxIpCH& KnxIpTunnelingAck::connectionHeader()
{
    return _ch;
}
#endif