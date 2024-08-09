#include "knx_ip_connect_response.h"
#ifdef USE_IP

KnxIpConnectResponse::KnxIpConnectResponse(IpParameterObject& parameters, uint16_t address, uint16_t port, uint8_t channel, uint8_t type)
    : KnxIpFrame(LEN_KNXIP_HEADER + 1 /*Channel*/ + 1 /*Status*/ + LEN_IPHPAI + ((type == 4) ? 4 : 2)),
      _controlEndpoint(_data + LEN_KNXIP_HEADER + 1 /*Channel*/ + 1 /*Status*/),
      _crd(_data + LEN_KNXIP_HEADER + 1 /*Channel*/ + 1 /*Status*/ + LEN_IPHPAI)
{
    serviceTypeIdentifier(ConnectResponse);

    _data[LEN_KNXIP_HEADER] = channel;
    
    _controlEndpoint.length(LEN_IPHPAI);
    _controlEndpoint.code(IPV4_UDP);
    _controlEndpoint.ipAddress(parameters.propertyValue<uint32_t>(PID_CURRENT_IP_ADDRESS));
    _controlEndpoint.ipPortNumber(KNXIP_MULTICAST_PORT);

    _crd.length((type == 4) ? 4 : 2); //TunnelConnectionResponse length = 4; ConfigConnectionResponse length = 2;
    _crd.type(type);
    _crd.address(address);
}

KnxIpConnectResponse::KnxIpConnectResponse(uint8_t channel, uint8_t errorCode)
    : KnxIpFrame(LEN_KNXIP_HEADER + 1 /*Channel*/ + 1 /*Status*/),
      _controlEndpoint(nullptr),
      _crd(nullptr)
{
    serviceTypeIdentifier(ConnectResponse);

    _data[LEN_KNXIP_HEADER] = channel;
    _data[LEN_KNXIP_HEADER + 1] = errorCode;
}


IpHostProtocolAddressInformation& KnxIpConnectResponse::controlEndpoint()
{
    return _controlEndpoint;
}

KnxIpCRD& KnxIpConnectResponse::crd()
{
    return _crd;
}

#endif
