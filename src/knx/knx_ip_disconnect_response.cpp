#include "knx_ip_disconnect_response.h"
#ifdef USE_IP

KnxIpDisconnectResponse::KnxIpDisconnectResponse(uint8_t channel, uint8_t status)
    : KnxIpFrame(LEN_KNXIP_HEADER + 1 /*Channel*/ + 1 /*Status*/)
{
    serviceTypeIdentifier(DisconnectResponse);

    _data[LEN_KNXIP_HEADER] = channel;
    _data[LEN_KNXIP_HEADER+1] = status;
}
#endif
