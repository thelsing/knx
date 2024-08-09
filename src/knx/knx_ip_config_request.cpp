#include "knx_ip_config_request.h"
#ifdef USE_IP
KnxIpConfigRequest::KnxIpConfigRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _ch(data + LEN_KNXIP_HEADER), _frame(data + LEN_KNXIP_HEADER + LEN_CH, length - LEN_KNXIP_HEADER - LEN_CH)
{
}


CemiFrame& KnxIpConfigRequest::frame()
{
    return _frame;
}
KnxIpCH& KnxIpConfigRequest::connectionHeader()
{
    return _ch;
}
#endif