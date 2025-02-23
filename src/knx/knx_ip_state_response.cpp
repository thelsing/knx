#include "knx_ip_state_response.h"
#ifdef USE_IP

#define LEN_SERVICE_FAMILIES 2
#if MASK_VERSION == 0x091A
#ifdef KNX_TUNNELING
#define LEN_SERVICE_DIB (2 + 4 * LEN_SERVICE_FAMILIES)
#else
#define LEN_SERVICE_DIB (2 + 3 * LEN_SERVICE_FAMILIES)
#endif
#else
#ifdef KNX_TUNNELING
#define LEN_SERVICE_DIB (2 + 3 * LEN_SERVICE_FAMILIES)
#else
#define LEN_SERVICE_DIB (2 + 2 * LEN_SERVICE_FAMILIES)
#endif
#endif

KnxIpStateResponse::KnxIpStateResponse(uint8_t channelId, uint8_t errorCode)
    : KnxIpFrame(LEN_KNXIP_HEADER + 2)
{
    serviceTypeIdentifier(ConnectionStateResponse);
    _data[LEN_KNXIP_HEADER] = channelId;
    _data[LEN_KNXIP_HEADER + 1] = errorCode;
}
#endif
