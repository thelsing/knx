#pragma once

#include "ip_host_protocol_address_information.h"
#include "ip_parameter_object.h"
#include "knx_ip_device_information_dib.h"
#include "knx_ip_frame.h"
#include "knx_ip_supported_service_dib.h"
#include "service_families.h"

namespace Knx
{
    class KnxIpSearchResponse : public KnxIpFrame
    {
        public:
            KnxIpSearchResponse(IpParameterObject& parameters, DeviceObject& deviceObj);
            IpHostProtocolAddressInformation& controlEndpoint();
            KnxIpDeviceInformationDIB& deviceInfo();
            KnxIpSupportedServiceDIB& supportedServices();

        private:
            IpHostProtocolAddressInformation _controlEndpoint;
            KnxIpDeviceInformationDIB _deviceInfo;
            KnxIpSupportedServiceDIB _supportedServices;
    };
} // namespace Knx