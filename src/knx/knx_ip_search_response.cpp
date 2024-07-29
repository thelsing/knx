#include "knx_ip_search_response.h"
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

KnxIpSearchResponse::KnxIpSearchResponse(IpParameterObject& parameters, DeviceObject& deviceObject)
    : KnxIpFrame(LEN_KNXIP_HEADER + LEN_IPHPAI + LEN_DEVICE_INFORMATION_DIB + LEN_SERVICE_DIB),
      _controlEndpoint(_data + LEN_KNXIP_HEADER), _deviceInfo(_data + LEN_KNXIP_HEADER + LEN_IPHPAI),
      _supportedServices(_data + LEN_KNXIP_HEADER + LEN_IPHPAI + LEN_DEVICE_INFORMATION_DIB)
{
    serviceTypeIdentifier(SearchResponse);

    _controlEndpoint.length(LEN_IPHPAI);
    _controlEndpoint.code(IPV4_UDP);
    _controlEndpoint.ipAddress(parameters.propertyValue<uint32_t>(PID_CURRENT_IP_ADDRESS));
    _controlEndpoint.ipPortNumber(KNXIP_MULTICAST_PORT);

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
    _supportedServices.serviceVersion(Core, KNX_SERVICE_FAMILY_CORE);
    _supportedServices.serviceVersion(DeviceManagement, KNX_SERVICE_FAMILY_DEVICE_MANAGEMENT);
#ifdef KNX_TUNNELING
    _supportedServices.serviceVersion(Tunnelling, KNX_SERVICE_FAMILY_TUNNELING);
#endif
#if MASK_VERSION == 0x091A
    _supportedServices.serviceVersion(Routing, KNX_SERVICE_FAMILY_ROUTING);
#endif
}


IpHostProtocolAddressInformation& KnxIpSearchResponse::controlEndpoint()
{
    return _controlEndpoint;
}


KnxIpDeviceInformationDIB& KnxIpSearchResponse::deviceInfo()
{
    return _deviceInfo;
}


KnxIpSupportedServiceDIB& KnxIpSearchResponse::supportedServices()
{
    return _supportedServices;
}
#endif
