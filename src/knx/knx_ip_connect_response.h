#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_crd.h"
#include "ip_host_protocol_address_information.h"
#include "knx_ip_device_information_dib.h"
#include "knx_ip_supported_service_dib.h"
#include "ip_parameter_object.h"
#ifdef USE_IP

enum KnxIpConnectionRequestErrorCodes
{
  E_NO_ERROR = 0,

  E_HOST_PROTOCOL_TYPE = 0x01,
  E_VERSION_NOT_SUPPORTED = 0x02,
  E_SEQUENCE_NUMBER = 0x04,

  E_ERROR = 0x0F,
  
  E_CONNECTION_ID = 0x21,
  E_CONNECTION_TYPE = 0x22,
  E_CONNECTION_OPTION = 0x23,
  E_NO_MORE_CONNECTIONS = 0x24,
  E_DATA_CONNECTION = 0x26,
  E_KNX_CONNECTION = 0x27,
  E_AUTHORISATION_ERROR = 0x28,
  E_TUNNELING_LAYER = 0x29,
  E_NO_TUNNELLING_ADDRESS = 0x2D,
  E_CONNECTION_IN_USE = 0x2E
};

class KnxIpConnectResponse : public KnxIpFrame
{
  public:
    KnxIpConnectResponse(IpParameterObject& parameters, uint16_t address, uint16_t port, uint8_t channel, uint8_t type);
    KnxIpConnectResponse(uint8_t channel, uint8_t errorCode);
    IpHostProtocolAddressInformation& controlEndpoint();
    KnxIpCRD& crd();
  private:
    IpHostProtocolAddressInformation _controlEndpoint;
    KnxIpCRD _crd;
};

#endif