#include "knx_ip_frame.h"

#ifdef USE_IP
#include "bits.h"

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

KnxIpFrame::KnxIpFrame(uint8_t* data,
                       uint16_t length)
{
    _data = data;
}

uint8_t KnxIpFrame::headerLength() const
{
    return _data[0];
}

void KnxIpFrame::headerLength(uint8_t length)
{
    _data[0] = length;
}

KnxIpVersion KnxIpFrame::protocolVersion() const
{
    return (KnxIpVersion)_data[1];
}

void KnxIpFrame::protocolVersion(KnxIpVersion version)
{
    _data[1] = (uint8_t)version;
}

uint16_t KnxIpFrame::serviceTypeIdentifier() const
{
    return 0;
}

void KnxIpFrame::serviceTypeIdentifier(uint16_t identifier)
{
}

uint16_t KnxIpFrame::totalLength() const
{
    return getWord(_data + 2);
}

void KnxIpFrame::totalLength(uint16_t length)
{
    pushWord(length, _data + 2);
}
#endif