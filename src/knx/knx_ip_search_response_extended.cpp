#include "knx_ip_search_response_extended.h"
#include "service_families.h"
#if KNX_SERVICE_FAMILY_CORE >= 2
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

KnxIpSearchResponseExtended::KnxIpSearchResponseExtended(IpParameterObject& parameters, DeviceObject& deviceObject, int dibLength)
    : KnxIpFrame(LEN_KNXIP_HEADER + LEN_IPHPAI + dibLength),
      _controlEndpoint(_data + LEN_KNXIP_HEADER)
{
    serviceTypeIdentifier(SearchResponseExt);

    _controlEndpoint.length(LEN_IPHPAI);
    _controlEndpoint.code(IPV4_UDP);
    _controlEndpoint.ipAddress(parameters.propertyValue<uint32_t>(PID_CURRENT_IP_ADDRESS));
    _controlEndpoint.ipPortNumber(KNXIP_MULTICAST_PORT);

    currentPos = LEN_KNXIP_HEADER + LEN_IPHPAI;
}

void KnxIpSearchResponseExtended::setDeviceInfo(IpParameterObject& parameters, DeviceObject& deviceObject)
{
    println("setDeviceInfo");
    KnxIpDeviceInformationDIB _deviceInfo(_data + currentPos);
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

    currentPos += LEN_DEVICE_INFORMATION_DIB;
}

void KnxIpSearchResponseExtended::setSupportedServices()
{
    println("setSupportedServices");
    KnxIpSupportedServiceDIB _supportedServices(_data + currentPos);
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
    currentPos += LEN_SERVICE_DIB;
}

void KnxIpSearchResponseExtended::setIpConfig(IpParameterObject& parameters)
{
    println("setIpConfig");
    KnxIpConfigDIB _ipConfig(_data + currentPos);
    _ipConfig.length(LEN_IP_CONFIG_DIB);
    _ipConfig.code(IP_CONFIG);
    _ipConfig.address(parameters.propertyValue<uint32_t>(PID_IP_ADDRESS));
    _ipConfig.subnet(parameters.propertyValue<uint32_t>(PID_SUBNET_MASK));
    _ipConfig.gateway(parameters.propertyValue<uint32_t>(PID_DEFAULT_GATEWAY));
    _ipConfig.info1(parameters.propertyValue<uint8_t>(PID_IP_CAPABILITIES));
    _ipConfig.info2(parameters.propertyValue<uint8_t>(PID_IP_ASSIGNMENT_METHOD));

    currentPos += LEN_IP_CONFIG_DIB;
}

void KnxIpSearchResponseExtended::setIpCurrentConfig(IpParameterObject& parameters)
{
    println("setIpCurrentConfig");
    KnxIpConfigDIB _ipCurConfig(_data + currentPos, true);
    _ipCurConfig.length(LEN_IP_CURRENT_CONFIG_DIB);
    _ipCurConfig.code(IP_CUR_CONFIG);
    _ipCurConfig.address(parameters.propertyValue<uint32_t>(PID_CURRENT_IP_ADDRESS));
    _ipCurConfig.subnet(parameters.propertyValue<uint32_t>(PID_CURRENT_SUBNET_MASK));
    _ipCurConfig.gateway(parameters.propertyValue<uint32_t>(PID_CURRENT_DEFAULT_GATEWAY));
    _ipCurConfig.dhcp(parameters.propertyValue<uint32_t>(PID_DHCP_BOOTP_SERVER));
    _ipCurConfig.info1(parameters.propertyValue<uint8_t>(PID_CURRENT_IP_ASSIGNMENT_METHOD));
    _ipCurConfig.info2(0x00); //Reserved

    currentPos += LEN_IP_CURRENT_CONFIG_DIB;
}

void KnxIpSearchResponseExtended::setKnxAddresses(IpParameterObject& parameters, DeviceObject& deviceObject)
{
    println("setKnxAddresses");
    KnxIpKnxAddressesDIB _knxAddresses(_data + currentPos);
    _knxAddresses.length(4); //minimum
    _knxAddresses.code(KNX_ADDRESSES);
    _knxAddresses.individualAddress(deviceObject.individualAddress());

    uint16_t length = 0;
    parameters.readPropertyLength(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES, length);

    const uint8_t *addresses = parameters.propertyData(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES);

    for(int i = 0; i < length; i++)
    {
        uint16_t additional = 0;
        popWord(additional, addresses + i*2);
        _knxAddresses.additional(additional);
    }

    currentPos += _knxAddresses.length();
}

void KnxIpSearchResponseExtended::setTunnelingInfo(IpParameterObject& parameters, DeviceObject& deviceObject, KnxIpTunnelConnection tunnels[])
{
    println("setTunnelingInfo");
    KnxIpTunnelingInfoDIB _tunnelInfo(_data + currentPos);
    _tunnelInfo.length(4); //minlength
    _tunnelInfo.code(TUNNELING_INFO);
    _tunnelInfo.apduLength(254); //FIXME where to get from

    uint16_t length = 0;
    parameters.readPropertyLength(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES, length);
    
    const uint8_t *addresses;
    if(length == KNX_TUNNELING)
    {
        addresses = parameters.propertyData(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES);
    } else {
        uint8_t addrbuffer[KNX_TUNNELING*2];
        addresses = (uint8_t*)addrbuffer;
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            addrbuffer[i*2+1] = i+1;
            addrbuffer[i*2] = deviceObject.individualAddress() / 0x0100;
        }
    }

    for(int i = 0; i < length; i++)
    {
        uint16_t additional = 0;
        popWord(additional, addresses + i*2);
        uint16_t flags = 0;

        uint8_t doubleCounter = 0;
        bool used = false;
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            if(tunnels[i].IndividualAddress == additional)
            {
                doubleCounter += 1;
                if(tunnels[i].ChannelId != 0)
                    used = true;
            }
        }

        if(doubleCounter > 1 && used)
            flags |= 1 << 2; //Slot is not usable; double PA is already used

        if(used)
        {
            flags |= 1 << 2; //Slot is not usable; PA is already used
            flags |= 1; //Slot is not free
        }

        flags = ~flags;

        _tunnelInfo.tunnelingSlot(additional, flags);
    }

    currentPos += _tunnelInfo.length();
}

void KnxIpSearchResponseExtended::setExtendedDeviceInfo()
{
    println("setExtendedDeviceInfo");
    KnxIpExtendedDeviceInformationDIB _extended(_data + currentPos);
    _extended.length(LEN_EXTENDED_DEVICE_INFORMATION_DIB);
    _extended.code(EXTENDED_DEVICE_INFO);
    _extended.status(0x01); //FIXME dont know encoding PID_MEDIUM_STATUS=51 RouterObject
    _extended.localMaxApdu(254); //FIXME is this correct?
    _extended.deviceDescriptor(MASK_VERSION);

    currentPos += LEN_EXTENDED_DEVICE_INFORMATION_DIB;
}

IpHostProtocolAddressInformation& KnxIpSearchResponseExtended::controlEndpoint()
{
    return _controlEndpoint;
}


uint8_t *KnxIpSearchResponseExtended::DIBs()
{
    return _data + LEN_KNXIP_HEADER + LEN_IPHPAI;
}
#endif
#endif