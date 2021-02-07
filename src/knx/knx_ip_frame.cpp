#include "knx_ip_frame.h"

#ifdef USE_IP

#include <cstring>
#include "bits.h"

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

KnxIpFrame::KnxIpFrame(uint8_t* data,
                       uint16_t length)
{
    _data = data;
    _dataLength = length;
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
    return getWord(_data + 2);
}

void KnxIpFrame::serviceTypeIdentifier(uint16_t identifier)
{
    pushWord(identifier, _data + 2);
}

uint16_t KnxIpFrame::totalLength() const
{
    return getWord(_data + 4);
}

void KnxIpFrame::totalLength(uint16_t length)
{
    pushWord(length, _data + 4);
}

uint8_t* KnxIpFrame::data()
{
    return _data;
}


KnxIpFrame::~KnxIpFrame()
{
    if (_freeData)
        delete[] _data;
}


KnxIpFrame::KnxIpFrame(uint16_t length)
{
    _data = new uint8_t[length];
    _dataLength = length;
    _freeData = true;
    memset(_data, 0, length);
    headerLength(LEN_KNXIP_HEADER);
    protocolVersion(KnxIp1_0);
    totalLength(length);
}
#endif