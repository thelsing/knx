#pragma once

#include "service_families.h"

#include "ip_host_protocol_address_information.h"
#include "ip_parameter_object.h"
#include "knx_ip_config_dib.h"
#include "knx_ip_device_information_dib.h"
#include "knx_ip_extended_device_information_dib.h"
#include "knx_ip_frame.h"
#include "knx_ip_knx_addresses_dib.h"
#include "knx_ip_supported_service_dib.h"
#include "knx_ip_tunnel_connection.h"
#include "knx_ip_tunneling_info_dib.h"

namespace Knx
{
    class KnxIpSearchResponseExtended : public KnxIpFrame
    {
        public:
            KnxIpSearchResponseExtended(IpParameterObject& parameters, DeviceObject& deviceObj, int dibLength);
            IpHostProtocolAddressInformation& controlEndpoint();
            void setDeviceInfo(IpParameterObject& parameters, DeviceObject& deviceObject);
            void setSupportedServices();
            void setIpConfig(IpParameterObject& parameters);
            void setIpCurrentConfig(IpParameterObject& parameters);
            void setKnxAddresses(IpParameterObject& parameters, DeviceObject& deviceObject);
            // setManuData
            void setTunnelingInfo(IpParameterObject& parameters, DeviceObject& deviceObject, KnxIpTunnelConnection tunnels[]);
            void setExtendedDeviceInfo();
            uint8_t* DIBs();

        private:
            IpHostProtocolAddressInformation _controlEndpoint;
            int currentPos = 0;
    };
} // namespace Knx