#include "knx_ip_search_response.h"
#ifdef USE_IP

#define SERVICE_FAMILIES 2

KnxIpSearchResponse::KnxIpSearchResponse(IpParameterObject& parameters, DeviceObject& deviceObject)
    : KnxIpFrame(LEN_KNXIP_HEADER + LEN_IPHPAI + LEN_DEVICE_INFORMATION_DIB + 2 + 2 * SERVICE_FAMILIES),
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
    _deviceInfo.medium(0x20); //KNX-IP FIXME get this value from somewhere else
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

    _supportedServices.length(2 + 2 * SERVICE_FAMILIES);
    _supportedServices.code(SUPP_SVC_FAMILIES);
    _supportedServices.serviceVersion(Core, 1);
    _supportedServices.serviceVersion(DeviceManagement, 1);
//    _supportedServices.serviceVersion(Routing, 1);
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
