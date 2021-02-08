#include "knx_ip_routing_indication.h"
#include <cstring>

#ifdef USE_IP
CemiFrame& KnxIpRoutingIndication::frame()
{
    return _frame;
}


KnxIpRoutingIndication::KnxIpRoutingIndication(uint8_t* data, 
    uint16_t length) : KnxIpFrame(data, length), _frame(data + headerLength(), length - headerLength())
{
}

KnxIpRoutingIndication::KnxIpRoutingIndication(CemiFrame frame)
    : KnxIpFrame(frame.totalLenght() + LEN_KNXIP_HEADER), _frame(_data + headerLength(), frame.totalLenght())
{
    serviceTypeIdentifier(RoutingIndication);
    memcpy(_data + LEN_KNXIP_HEADER, frame.data(), frame.totalLenght());
}
#endif