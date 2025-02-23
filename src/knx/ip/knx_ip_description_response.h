#pragma once

#include "ip_host_protocol_address_information.h"
#include "ip_parameter_object.h"
#include "knx_ip_device_information_dib.h"
#include "knx_ip_frame.h"
#include "knx_ip_supported_service_dib.h"

namespace Knx
{
    class KnxIpDescriptionResponse : public KnxIpFrame
    {
        public:
            KnxIpDescriptionResponse(IpParameterObject& parameters, DeviceObject& deviceObj);
            KnxIpDeviceInformationDIB& deviceInfo();
            KnxIpSupportedServiceDIB& supportedServices();

        private:
            KnxIpDeviceInformationDIB _deviceInfo;
            KnxIpSupportedServiceDIB _supportedServices;
    };
} // namespace Knx