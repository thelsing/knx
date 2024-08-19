#include "knx_ip_frame.h"

#include <cstring>
#include "../bits.h"

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

KnxIpServiceType KnxIpFrame::serviceTypeIdentifier() const
{
    return (KnxIpServiceType)getWord(_data + 2);
}

void KnxIpFrame::serviceTypeIdentifier(KnxIpServiceType identifier)
{
    pushWord((uint16_t) identifier, _data + 2);
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

const char* enum_name(const KnxIpVersion enum_val)
{
    switch (enum_val)
    {
        case KnxIp1_0:
            return "KnxIp1_0";
    }

    return "";
}

const char* enum_name(const KnxIpServiceType enum_val)
{
    switch (enum_val)
    {
        case SearchRequest:
            return "SearchRequest";

        case SearchResponse:
            return "SearchResponse";

        case DescriptionRequest:
            return "DescriptionRequest";

        case DescriptionResponse:
            return "DescriptionResponse";

        case ConnectRequest:
            return "ConnectRequest";

        case ConnectResponse:
            return "ConnectResponse";

        case ConnectionStateRequest:
            return "ConnectionStateRequest";

        case ConnectionStateResponse:
            return "ConnectionStateResponse";

        case DisconnectRequest:
            return "DisconnectRequest";

        case DisconnectResponse:
            return "DisconnectResponse";

        case SearchRequestExt:
            return "SearchRequestExt";

        case SearchResponseExt:
            return "SearchResponseExt";

        case DeviceConfigurationRequest:
            return "DeviceConfigurationRequest";

        case DeviceConfigurationAck:
            return "DeviceConfigurationAck";

        case TunnelingRequest:
            return "TunnelingRequest";

        case TunnelingAck:
            return "TunnelingAck";

        case RoutingIndication:
            return "RoutingIndication";

        case RoutingLostMessage:
            return "RoutingLostMessage";
    }

    return "";
}