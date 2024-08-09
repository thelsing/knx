#include "knx_ip_description_response.h"
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

KnxIpDescriptionResponse::KnxIpDescriptionResponse(IpParameterObject& parameters, DeviceObject& deviceObject)
    : KnxIpFrame(LEN_KNXIP_HEADER + LEN_DEVICE_INFORMATION_DIB + LEN_SERVICE_DIB),
      _deviceInfo(_data + LEN_KNXIP_HEADER),
      _supportedServices(_data + LEN_KNXIP_HEADER + LEN_DEVICE_INFORMATION_DIB)
{
    serviceTypeIdentifier(DescriptionResponse);

    _deviceInfo.length(LEN_DEVICE_INFORMATION_DIB);
    _deviceInfo.code(DEVICE_INFO);
#if MASK_VERSION == 0x57B0
    _deviceInfo.medium(0x20); //MediumType is IP (for IP-Only Devices)
#else
    _deviceInfo.medium(0x02); //MediumType is TP
#endif
    _deviceInfo.status(deviceObject.progMode());
    _deviceInfo.individualAddress(parameters.propertyValue<uint16_t>(PID_KNX_INDIVIDUAL_ADDRESS));
    _deviceInfo.projectInstallationIdentifier(parameters.propertyValue<uint16_t>(PID_PROJECT_INSTALLATION_ID));
    _deviceInfo.serialNumber(deviceObject.propertyData(PID_SERIAL_NUMBER));
    _deviceInfo.routingMulticastAddress(parameters.propertyValue<uint32_t>(PID_ROUTING_MULTICAST_ADDRESS));
    //_deviceInfo.routingMulticastAddress(0);

    uint8_t mac_address[LEN_MAC_ADDRESS] = {0};
    Property* prop = parameters.property(PID_MAC_ADDRESS);
    prop->read(mac_address);
    _deviceInfo.macAddress(mac_address);
    
    uint8_t friendlyName[LEN_FRIENDLY_NAME] = {0};
    prop = parameters.property(PID_FRIENDLY_NAME);
    prop->read(1, LEN_FRIENDLY_NAME, friendlyName);
    _deviceInfo.friendlyName(friendlyName);

    _supportedServices.length(LEN_SERVICE_DIB);
    _supportedServices.code(SUPP_SVC_FAMILIES);
    _supportedServices.serviceVersion(Core, 1);
    _supportedServices.serviceVersion(DeviceManagement, 1);
#ifdef KNX_TUNNELING
    _supportedServices.serviceVersion(Tunnelling, 1);
#endif
#if MASK_VERSION == 0x091A
    _supportedServices.serviceVersion(Routing, 1);
#endif
}

KnxIpDeviceInformationDIB& KnxIpDescriptionResponse::deviceInfo()
{
    return _deviceInfo;
}


KnxIpSupportedServiceDIB& KnxIpDescriptionResponse::supportedServices()
{
    return _supportedServices;
}
#endif
