#include "knx_ip_tunneling_request.h"
#include <cstring>

#ifdef USE_IP
KnxIpTunnelingRequest::KnxIpTunnelingRequest(uint8_t* data,
                                             uint16_t length)
    : KnxIpFrame(data, length), _frame(data + LEN_CH + headerLength(), length - LEN_CH - headerLength()), _ch(_data + headerLength())
{
}

KnxIpTunnelingRequest::KnxIpTunnelingRequest(CemiFrame frame)
    : KnxIpFrame(frame.totalLenght() + LEN_CH + LEN_KNXIP_HEADER), _frame(_data + LEN_CH + LEN_KNXIP_HEADER, frame.totalLenght()), _ch(_data + LEN_KNXIP_HEADER)
{
    serviceTypeIdentifier(TunnelingRequest);
    memcpy(_data + LEN_KNXIP_HEADER + LEN_CH, frame.data(), frame.totalLenght());
}

CemiFrame& KnxIpTunnelingRequest::frame()
{
    return _frame;
}

KnxIpCH& KnxIpTunnelingRequest::connectionHeader()
{
    return _ch;
}
#endif