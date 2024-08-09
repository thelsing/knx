#include "config.h"
#ifdef USE_IP

#include "ip_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "knx_ip_routing_indication.h"
#include "knx_ip_search_request.h"
#include "knx_ip_search_response.h"
#include "knx_ip_search_request_extended.h"
#include "knx_ip_search_response_extended.h"
#include "knx_facade.h"
#ifdef KNX_TUNNELING
#include "knx_ip_connect_request.h"
#include "knx_ip_connect_response.h"
#include "knx_ip_state_request.h"
#include "knx_ip_state_response.h"
#include "knx_ip_disconnect_request.h"
#include "knx_ip_disconnect_response.h"
#include "knx_ip_tunneling_request.h"
#include "knx_ip_tunneling_ack.h"
#include "knx_ip_description_request.h"
#include "knx_ip_description_response.h"
#include "knx_ip_config_request.h"
#endif

#include <stdio.h>
#include <string.h>

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

#define MIN_LEN_CEMI 10

IpDataLinkLayer::IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam,
    NetworkLayerEntity &netLayerEntity, Platform& platform, BusAccessUnit& busAccessUnit, DataLinkLayerCallbacks* dllcb) : DataLinkLayer(devObj, netLayerEntity, platform, busAccessUnit), _ipParameters(ipParam), _dllcb(dllcb)
{
}

bool IpDataLinkLayer::sendFrame(CemiFrame& frame)
{
    KnxIpRoutingIndication packet(frame);
    // only send 50 packet per second: see KNX 3.2.6 p.6
    if(isSendLimitReached())
        return false;
    bool success = sendBytes(packet.data(), packet.totalLength());
#ifdef KNX_ACTIVITYCALLBACK
    if(_dllcb)
        _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_SEND << KNX_ACTIVITYCALLBACK_DIR));
#endif
    dataConReceived(frame, success);
    return success;
}

#ifdef KNX_TUNNELING
void IpDataLinkLayer::dataRequestToTunnel(CemiFrame& frame)
{
    if(frame.addressType() == AddressType::GroupAddress)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
            if(tunnels[i].ChannelId != 0 && tunnels[i].IndividualAddress == frame.sourceAddress())
                sendFrameToTunnel(&tunnels[i], frame);
                //TODO check if source is from tunnel
        return;
    }

    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].IndividualAddress == frame.sourceAddress())
            continue;

        if(tunnels[i].IndividualAddress == frame.destinationAddress())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            if(tunnels[i].IsConfig)
            {
#ifdef KNX_LOG_TUNNELING
                println("Found config Channel");
#endif
                tun = &tunnels[i];
                break;
            }
        }
    }

    if(tun == nullptr)
    {
#ifdef KNX_LOG_TUNNELING
        print("Found no Tunnel for IA: ");
        println(frame.destinationAddress(), 16);
#endif
        return;
    }

    sendFrameToTunnel(tun, frame);
}

void IpDataLinkLayer::dataConfirmationToTunnel(CemiFrame& frame)
{
    if(frame.addressType() == AddressType::GroupAddress)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
            if(tunnels[i].ChannelId != 0 && tunnels[i].IndividualAddress == frame.sourceAddress())
                sendFrameToTunnel(&tunnels[i], frame);
                //TODO check if source is from tunnel
        return;
    }

    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].IndividualAddress == frame.destinationAddress())
            continue;
            
        if(tunnels[i].IndividualAddress == frame.sourceAddress())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            if(tunnels[i].IsConfig)
            {
#ifdef KNX_LOG_TUNNELING
                println("Found config Channel");
#endif
                tun = &tunnels[i];
                break;
            }
        }
    }

    if(tun == nullptr)
    {
#ifdef KNX_LOG_TUNNELING
        print("Found no Tunnel for IA: ");
        println(frame.destinationAddress(), 16);
#endif
        return;
    }

    sendFrameToTunnel(tun, frame);
}

void IpDataLinkLayer::dataIndicationToTunnel(CemiFrame& frame)
{
    if(frame.addressType() == AddressType::GroupAddress)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
            if(tunnels[i].ChannelId != 0 && tunnels[i].IndividualAddress != frame.sourceAddress())
                sendFrameToTunnel(&tunnels[i], frame);
        return;
    }

    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].ChannelId == 0 || tunnels[i].IndividualAddress == frame.sourceAddress())
            continue;
            
        if(tunnels[i].IndividualAddress == frame.destinationAddress())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            if(tunnels[i].IsConfig)
            {
#ifdef KNX_LOG_TUNNELING
                println("Found config Channel");
#endif
                tun = &tunnels[i];
                break;
            }
        }
    }

    if(tun == nullptr)
    {
#ifdef KNX_LOG_TUNNELING
        print("Found no Tunnel for IA: ");
        println(frame.destinationAddress(), 16);
#endif
        return;
    }

    sendFrameToTunnel(tun, frame);
}

void IpDataLinkLayer::sendFrameToTunnel(KnxIpTunnelConnection *tunnel, CemiFrame& frame)
{
#ifdef KNX_LOG_TUNNELING
    print("Send to Channel: ");
    println(tunnel->ChannelId, 16);
#endif
    KnxIpTunnelingRequest req(frame);
    req.connectionHeader().sequenceCounter(tunnel->SequenceCounter_S++);
    req.connectionHeader().length(LEN_CH);
    req.connectionHeader().channelId(tunnel->ChannelId);

    if(frame.messageCode() != L_data_req && frame.messageCode() != L_data_con && frame.messageCode() != L_data_ind)
        req.serviceTypeIdentifier(DeviceConfigurationRequest);

    _platform.sendBytesUniCast(tunnel->IpAddress, tunnel->PortData, req.data(), req.totalLength());
}

bool IpDataLinkLayer::isTunnelAddress(uint16_t addr)
{
    if(addr == 0)
        return false; // 0.0.0 is not a valid tunnel address and is used as default value
    
    for(int i = 0; i < KNX_TUNNELING; i++)
        if(tunnels[i].IndividualAddress == addr)
            return true;

    return false;
}

bool IpDataLinkLayer::isSentToTunnel(uint16_t address, bool isGrpAddr)
{
    if(isGrpAddr)
    {
        for(int i = 0; i < KNX_TUNNELING; i++)
            if(tunnels[i].ChannelId != 0)
                return true;
        return false;
    } else {
        for(int i = 0; i < KNX_TUNNELING; i++)
            if(tunnels[i].ChannelId != 0 && tunnels[i].IndividualAddress == address)
                return true;
        return false;
    }
}
#endif

void IpDataLinkLayer::loop()
{
    if (!_enabled)
        return;

#ifdef KNX_TUNNELING
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].ChannelId != 0)
        {
            if(millis() - tunnels[i].lastHeartbeat > 120000)
            {
    #ifdef KNX_LOG_TUNNELING
                print("Closed Tunnel 0x");
                print(tunnels[i].ChannelId, 16);
                println(" due to no heartbeat in 2 minutes");
    #endif
                KnxIpDisconnectRequest discReq;
                discReq.channelId(tunnels[i].ChannelId);
                discReq.hpaiCtrl().length(LEN_IPHPAI);
                discReq.hpaiCtrl().code(IPV4_UDP);
                discReq.hpaiCtrl().ipAddress(tunnels[i].IpAddress);
                discReq.hpaiCtrl().ipPortNumber(tunnels[i].PortCtrl);
                _platform.sendBytesUniCast(tunnels[i].IpAddress, tunnels[i].PortCtrl, discReq.data(), discReq.totalLength());
                tunnels[i].Reset();
            }
            break;
        }
    }
#endif


    uint8_t buffer[512];
    uint16_t remotePort = 0;
    uint32_t remoteAddr = 0;
    int len = _platform.readBytesMultiCast(buffer, 512, remoteAddr, remotePort);
    if (len <= 0)
        return;

    if (len < KNXIP_HEADER_LEN)
        return;
    
    if (buffer[0] != KNXIP_HEADER_LEN 
        || buffer[1] != KNXIP_PROTOCOL_VERSION)
        return;

#ifdef KNX_ACTIVITYCALLBACK
    if(_dllcb)
        _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_RECV << KNX_ACTIVITYCALLBACK_DIR));
#endif

    uint16_t code;
    popWord(code, buffer + 2);
    switch ((KnxIpServiceType)code)
    {
        case RoutingIndication:
        {
            KnxIpRoutingIndication routingIndication(buffer, len);
            frameReceived(routingIndication.frame());
            break;
        }
        
        case SearchRequest:
        {
            KnxIpSearchRequest searchRequest(buffer, len);
            KnxIpSearchResponse searchResponse(_ipParameters, _deviceObject);

            auto hpai = searchRequest.hpai();
#ifdef KNX_ACTIVITYCALLBACK
            if(_dllcb)
                _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_SEND << KNX_ACTIVITYCALLBACK_DIR) | (KNX_ACTIVITYCALLBACK_IPUNICAST));
#endif
            _platform.sendBytesUniCast(hpai.ipAddress(), hpai.ipPortNumber(), searchResponse.data(), searchResponse.totalLength());
            break;
        }
        case SearchRequestExt:
        {
            #if KNX_SERVICE_FAMILY_CORE >= 2
            loopHandleSearchRequestExtended(buffer, len);
            #endif
            break;
        }
#ifdef KNX_TUNNELING
        case ConnectRequest:
        {
            loopHandleConnectRequest(buffer, len, remoteAddr, remotePort);
            break;
        }

        case ConnectionStateRequest:
        {
            loopHandleConnectionStateRequest(buffer, len);
            break;
        }

        case DisconnectRequest:
        {
            loopHandleDisconnectRequest(buffer, len);
            break;
        }

        case DescriptionRequest:
        {
            loopHandleDescriptionRequest(buffer, len);
            break;
        }

        case DeviceConfigurationRequest:
        {
            loopHandleDeviceConfigurationRequest(buffer, len);
            break;
        }

        case TunnelingRequest:
        {
            loopHandleTunnelingRequest(buffer, len);
            return;
        }

        case DeviceConfigurationAck:
        {
            //TOOD nothing to do now
            //println("got Ack");
            break;
        }

        case TunnelingAck:
        {
            //TOOD nothing to do now
            //println("got Ack");
            break;
        }
#endif
        default:
            print("Unhandled service identifier: ");
            println(code, HEX);
            break;
    }
}

#if KNX_SERVICE_FAMILY_CORE >= 2
void IpDataLinkLayer::loopHandleSearchRequestExtended(uint8_t* buffer, uint16_t length)
{
    KnxIpSearchRequestExtended searchRequest(buffer, length);

    if(searchRequest.srpByProgMode)
    {
        println("srpByProgMode");
        if(!knx.progMode()) return;
    }

    if(searchRequest.srpByMacAddr)
    {
        println("srpByMacAddr");
        const uint8_t *x = _ipParameters.propertyData(PID_MAC_ADDRESS);
        for(int i = 0; i<6;i++)
            if(searchRequest.srpMacAddr[i] != x[i])
                return;
    }

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

    //defaults: "Device Information DIB", "Extended Device Information DIB" and "Supported Services DIB".
    int dibLength = LEN_DEVICE_INFORMATION_DIB + LEN_SERVICE_DIB + LEN_EXTENDED_DEVICE_INFORMATION_DIB;

    if(searchRequest.srpByService)
    {
        println("srpByService");
        uint8_t length = searchRequest.srpServiceFamilies[0];
        uint8_t *currentPos = searchRequest.srpServiceFamilies + 2;
        for(int i = 0; i < (length-2)/2; i++)
        {
            uint8_t serviceFamily = (currentPos + i*2)[0];
            uint8_t version = (currentPos + i*2)[1];
            switch(serviceFamily)
            {
                case Core:
                    if(version > KNX_SERVICE_FAMILY_CORE) return;
                    break;
                case DeviceManagement:
                    if(version > KNX_SERVICE_FAMILY_DEVICE_MANAGEMENT) return;
                    break;
                case Tunnelling:
                    if(version > KNX_SERVICE_FAMILY_TUNNELING) return;
                    break;
                case Routing:
                    if(version > KNX_SERVICE_FAMILY_ROUTING) return;
                    break;
            }
        }
    }

    if(searchRequest.srpRequestDIBs)
    {
        println("srpRequestDIBs");
        if(searchRequest.requestedDIB(IP_CONFIG))
            dibLength += LEN_IP_CONFIG_DIB; //16

        if(searchRequest.requestedDIB(IP_CUR_CONFIG))
            dibLength += LEN_IP_CURRENT_CONFIG_DIB; //20

        if(searchRequest.requestedDIB(KNX_ADDRESSES))
        {uint16_t length = 0;
            _ipParameters.readPropertyLength(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES, length);
            dibLength += 4 + length*2;
        }

        if(searchRequest.requestedDIB(MANUFACTURER_DATA))
            dibLength += 0; //4 + n

        if(searchRequest.requestedDIB(TUNNELING_INFO))
        {
            uint16_t length = 0;
            _ipParameters.readPropertyLength(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES, length);
            dibLength += 4 + length*4;
        }
    }

    KnxIpSearchResponseExtended searchResponse(_ipParameters, _deviceObject, dibLength);

    searchResponse.setDeviceInfo(_ipParameters, _deviceObject); //DescriptionTypeCode::DeviceInfo 1
    searchResponse.setSupportedServices(); //DescriptionTypeCode::SUPP_SVC_FAMILIES 2
    searchResponse.setExtendedDeviceInfo(); //DescriptionTypeCode::EXTENDED_DEVICE_INFO 8

    if(searchRequest.srpRequestDIBs)
    {
        if(searchRequest.requestedDIB(IP_CONFIG))
            searchResponse.setIpConfig(_ipParameters);

        if(searchRequest.requestedDIB(IP_CUR_CONFIG))
            searchResponse.setIpCurrentConfig(_ipParameters);

        if(searchRequest.requestedDIB(KNX_ADDRESSES))
            searchResponse.setKnxAddresses(_ipParameters, _deviceObject);

        if(searchRequest.requestedDIB(MANUFACTURER_DATA))
        {
            //println("requested MANUFACTURER_DATA but not implemented");
        }

        if(searchRequest.requestedDIB(TUNNELING_INFO))
            searchResponse.setTunnelingInfo(_ipParameters, _deviceObject, tunnels);
    }

    if(searchResponse.totalLength() > 150)
    {
        println("skipped response cause length is not plausible");
        return;
    }

    _platform.sendBytesUniCast(searchRequest.hpai().ipAddress(), searchRequest.hpai().ipPortNumber(), searchResponse.data(), searchResponse.totalLength());
}
#endif

#ifdef KNX_TUNNELING
void IpDataLinkLayer::loopHandleConnectRequest(uint8_t* buffer, uint16_t length, uint32_t& src_addr, uint16_t& src_port)
{
    KnxIpConnectRequest connRequest(buffer, length);
#ifdef KNX_LOG_TUNNELING
    println("Got Connect Request!");
    switch(connRequest.cri().type())
    {
        case DEVICE_MGMT_CONNECTION:
            println("Device Management Connection");
            break;
        case TUNNEL_CONNECTION:
            println("Tunnel Connection");
            break;
        case REMLOG_CONNECTION:
            println("RemLog Connection");
            break;
        case REMCONF_CONNECTION:
            println("RemConf Connection");
            break;
        case OBJSVR_CONNECTION:
            println("ObjectServer Connection");
            break;
    }
    
    print("Data Endpoint: ");
    uint32_t ip = connRequest.hpaiData().ipAddress();
    print(ip >> 24);
    print(".");
    print((ip >> 16) & 0xFF);
    print(".");
    print((ip >> 8) & 0xFF);
    print(".");
    print(ip & 0xFF);
    print(":");
    println(connRequest.hpaiData().ipPortNumber());
    print("Ctrl Endpoint: ");
    ip = connRequest.hpaiCtrl().ipAddress();
    print(ip >> 24);
    print(".");
    print((ip >> 16) & 0xFF);
    print(".");
    print((ip >> 8) & 0xFF);
    print(".");
    print(ip & 0xFF);
    print(":");
    println(connRequest.hpaiCtrl().ipPortNumber());
#endif

    //We only support 0x03 and 0x04!
    if(connRequest.cri().type() != TUNNEL_CONNECTION && connRequest.cri().type() != DEVICE_MGMT_CONNECTION)
    {
#ifdef KNX_LOG_TUNNELING
        println("Only Tunnel/DeviceMgmt Connection ist supported!");
#endif
        KnxIpConnectResponse connRes(0x00, E_CONNECTION_TYPE);
        _platform.sendBytesUniCast(connRequest.hpaiCtrl().ipAddress(), connRequest.hpaiCtrl().ipPortNumber(), connRes.data(), connRes.totalLength());
        return;
    }

    if(connRequest.cri().type() == TUNNEL_CONNECTION && connRequest.cri().layer() != 0x02) //LinkLayer
    {
        //We only support 0x02!
#ifdef KNX_LOG_TUNNELING
    println("Only LinkLayer ist supported!");
#endif
        KnxIpConnectResponse connRes(0x00, E_TUNNELING_LAYER);
        _platform.sendBytesUniCast(connRequest.hpaiCtrl().ipAddress(), connRequest.hpaiCtrl().ipPortNumber(), connRes.data(), connRes.totalLength());
        return;
    }

    // data preparation

    uint32_t srcIP = connRequest.hpaiCtrl().ipAddress()? connRequest.hpaiCtrl().ipAddress() : src_addr;
    uint16_t srcPort = connRequest.hpaiCtrl().ipPortNumber()? connRequest.hpaiCtrl().ipPortNumber() : src_port;

    // read current elements in PID_ADDITIONAL_INDIVIDUAL_ADDRESSES 
    uint16_t propCount = 0;
    _ipParameters.readPropertyLength(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES, propCount);
    const uint8_t *addresses;
    if(propCount == KNX_TUNNELING)
    {
        addresses = _ipParameters.propertyData(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES);
    }
    else    // no tunnel PA configured, that means device is unconfigured and has 15.15.0
    {
        uint8_t addrbuffer[KNX_TUNNELING*2];
        addresses = (uint8_t*)addrbuffer;
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            addrbuffer[i*2+1] = i+1;
            addrbuffer[i*2] = _deviceObject.individualAddress() / 0x0100;
        }
        uint8_t count = KNX_TUNNELING;
        _ipParameters.writeProperty(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES, 1, addrbuffer, count);
#ifdef KNX_LOG_TUNNELING
    	println("no Tunnel-PAs configured, using own subnet");
#endif
    }

    _ipParameters.readPropertyLength(PID_CUSTOM_RESERVED_TUNNELS_CTRL, propCount);
    const uint8_t *tunCtrlBytes = nullptr;
    if(propCount == KNX_TUNNELING)
        tunCtrlBytes = _ipParameters.propertyData(PID_CUSTOM_RESERVED_TUNNELS_CTRL);

    _ipParameters.readPropertyLength(PID_CUSTOM_RESERVED_TUNNELS_IP, propCount);
    const uint8_t *tunCtrlIp = nullptr;
    if(propCount == KNX_TUNNELING)
        tunCtrlIp = _ipParameters.propertyData(PID_CUSTOM_RESERVED_TUNNELS_IP);
    
    bool resTunActive = (tunCtrlBytes && tunCtrlIp);
#ifdef KNX_LOG_TUNNELING
    	if(resTunActive) println("Reserved Tunnel Feature active");

        if(tunCtrlBytes)
            printHex("tunCtrlBytes", tunCtrlBytes, KNX_TUNNELING);
        if(tunCtrlIp)
            printHex("tunCtrlIp", tunCtrlIp, KNX_TUNNELING*4);
#endif

    // check if there is a reserved tunnel for the source
    int firstFreeTunnel = -1;
    int firstResAndFreeTunnel = -1;
    int firstResAndOccTunnel = -1;
    bool tunnelResActive[KNX_TUNNELING];
    uint8_t tunnelResOptions[KNX_TUNNELING];
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(resTunActive)
        {
            tunnelResActive[i] = *(tunCtrlBytes+i) & 0x80;
            tunnelResOptions[i] = (*(tunCtrlBytes+i) & 0x60) >> 5;
        }


        if(tunnelResActive[i])   // tunnel reserve feature active for this tunnel
        {
            #ifdef KNX_LOG_TUNNELING
            print("tunnel reserve feature active for this tunnel: ");
            print(tunnelResActive[i]);
            print("  options: ");
            println(tunnelResOptions[i]);
            #endif
            
            uint32_t rIP = 0;
            popInt(rIP, tunCtrlIp+4*i);
            if(srcIP == rIP && connRequest.cri().type() == TUNNEL_CONNECTION)
            {
                // reserved tunnel for this ip found
                if(tunnels[i].ChannelId == 0) // check if it is free
                {
                    if(firstResAndFreeTunnel < 0)
                        firstResAndFreeTunnel = i;
                }
                else
                {
                    if(firstResAndOccTunnel < 0)
                        firstResAndOccTunnel = i;
                }
            }
        }
        else
        {
            if(tunnels[i].ChannelId == 0 && firstFreeTunnel < 0)
                firstFreeTunnel = i;
        }
    }
#ifdef KNX_LOG_TUNNELING
    	print("firstFreeTunnel: ");
        print(firstFreeTunnel);
        print(" firstResAndFreeTunnel: ");
        print(firstResAndFreeTunnel);
        print(" firstResAndOccTunnel: ");
        println(firstResAndOccTunnel);
#endif
    
    
    uint8_t tunIdx = 0xff;
    if(resTunActive & (firstResAndFreeTunnel >= 0 || firstResAndOccTunnel >= 0))   // tunnel reserve feature active (for this src)
    {
        if(firstResAndFreeTunnel >= 0)
        {
            tunIdx = firstResAndFreeTunnel;
        }
        else if(firstResAndOccTunnel >= 0)
        {
            if(tunnelResOptions[firstResAndOccTunnel] == 1) // decline req
            {
                ; // do nothing => decline
            }
            else if(tunnelResOptions[firstResAndOccTunnel] == 2)  // close current tunnel connection on this tunnel and assign to this request
            {
                KnxIpDisconnectRequest discReq;
                discReq.channelId(tunnels[firstResAndOccTunnel].ChannelId);
                discReq.hpaiCtrl().length(LEN_IPHPAI);
                discReq.hpaiCtrl().code(IPV4_UDP);
                discReq.hpaiCtrl().ipAddress(tunnels[firstResAndOccTunnel].IpAddress);
                discReq.hpaiCtrl().ipPortNumber(tunnels[firstResAndOccTunnel].PortCtrl);
                _platform.sendBytesUniCast(tunnels[firstResAndOccTunnel].IpAddress, tunnels[firstResAndOccTunnel].PortCtrl, discReq.data(), discReq.totalLength());
                tunnels[firstResAndOccTunnel].Reset();


                tunIdx = firstResAndOccTunnel;
            }
            else if(tunnelResOptions[firstResAndOccTunnel] == 3)  // use the first unreserved tunnel (if one)
            {
                if(firstFreeTunnel >= 0)
                    tunIdx = firstFreeTunnel;
                else
                    ; // do nothing => decline
            }
            //else
                // should not happen
                // do nothing => decline
        }
        //else
            // should not happen
            // do nothing => decline
    }
    else
    {
        if(firstFreeTunnel >= 0)
            tunIdx = firstFreeTunnel;
        //else
        // do nothing => decline
    }

    KnxIpTunnelConnection *tun = nullptr;
    if(tunIdx != 0xFF)
    {
        tun = &tunnels[tunIdx];

        uint16_t tunPa = 0;
        popWord(tunPa, addresses + (tunIdx*2));

        //check if this PA is in use (should not happen, only when there is one pa wrongly assigned to more then one tunnel)
        for(int x = 0; x < KNX_TUNNELING; x++)
            if(tunnels[x].IndividualAddress == tunPa)
            {
#ifdef KNX_LOG_TUNNELING
    	        println("cannot use tunnel because PA is already in use");
#endif
                tunIdx = 0xFF;
                tun = nullptr;
                break;
            }
        
        tun->IndividualAddress = tunPa;

    }

    if(tun == nullptr)
    {
        println("no free tunnel availible");
        KnxIpConnectResponse connRes(0x00, E_NO_MORE_CONNECTIONS);
        _platform.sendBytesUniCast(connRequest.hpaiCtrl().ipAddress(), connRequest.hpaiCtrl().ipPortNumber(), connRes.data(), connRes.totalLength());
        return;
    }

    if(connRequest.cri().type() == DEVICE_MGMT_CONNECTION)
        tun->IsConfig = true;

    // the channel ID shall be unique on this tunnel server. catch the rare case of a double channel ID
    bool channelIdInUse;
    do
    {
        _lastChannelId++;
        channelIdInUse = false;
        for(int x = 0; x < KNX_TUNNELING; x++)
            if(tunnels[x].ChannelId == _lastChannelId)
                channelIdInUse = true;
    }
    while(channelIdInUse);

    tun->ChannelId = _lastChannelId;
    tun->lastHeartbeat = millis();
    if(_lastChannelId == 255)
        _lastChannelId = 0;

    tun->IpAddress = srcIP;
    tun->PortData = srcPort;
    tun->PortCtrl = connRequest.hpaiCtrl().ipPortNumber()?connRequest.hpaiCtrl().ipPortNumber():srcPort;

    print("New Tunnel-Connection[");
    print(tunIdx);
    print("], Channel: 0x");
    print(tun->ChannelId, 16);
    print(" PA: ");
    print(tun->IndividualAddress >> 12);
    print(".");
    print((tun->IndividualAddress >> 8) & 0xF);
    print(".");
    print(tun->IndividualAddress & 0xFF);

    print(" with ");
    print(tun->IpAddress >> 24);
    print(".");
    print((tun->IpAddress >> 16) & 0xFF);
    print(".");
    print((tun->IpAddress >> 8) & 0xFF);
    print(".");
    print(tun->IpAddress & 0xFF);
    print(":");
    print(tun->PortData);
    if(tun->PortData != tun->PortCtrl)
    {
        print(" (Ctrlport: ");
        print(tun->PortCtrl);
        print(")");
    }
    if(tun->IsConfig)
    {
        print(" (Config-Channel)");
    }
    println();


    KnxIpConnectResponse connRes(_ipParameters, tun->IndividualAddress, 3671, tun->ChannelId, connRequest.cri().type());
    _platform.sendBytesUniCast(tun->IpAddress, tun->PortCtrl, connRes.data(), connRes.totalLength());
}

void IpDataLinkLayer::loopHandleConnectionStateRequest(uint8_t* buffer, uint16_t length)
{
    KnxIpStateRequest stateRequest(buffer, length);

    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].ChannelId == stateRequest.channelId())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
#ifdef KNX_LOG_TUNNELING
        print("Channel ID nicht gefunden: ");
        println(stateRequest.channelId());
#endif
        KnxIpStateResponse stateRes(0x00, E_CONNECTION_ID);
        _platform.sendBytesUniCast(stateRequest.hpaiCtrl().ipAddress(), stateRequest.hpaiCtrl().ipPortNumber(), stateRes.data(), stateRes.totalLength());
        return;
    }

    //TODO check knx connection!
    //if no connection return E_KNX_CONNECTION

    //TODO check when to send E_DATA_CONNECTION

    tun->lastHeartbeat = millis();
    KnxIpStateResponse stateRes(tun->ChannelId, E_NO_ERROR);
    _platform.sendBytesUniCast(stateRequest.hpaiCtrl().ipAddress(), stateRequest.hpaiCtrl().ipPortNumber(), stateRes.data(), stateRes.totalLength());
}

void IpDataLinkLayer::loopHandleDisconnectRequest(uint8_t* buffer, uint16_t length)
{
    KnxIpDisconnectRequest discReq(buffer, length);
            
#ifdef KNX_LOG_TUNNELING
    print(">>> Disconnect Channel ID: ");
    println(discReq.channelId());
#endif
    
    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].ChannelId == discReq.channelId())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
#ifdef KNX_LOG_TUNNELING
        print("Channel ID nicht gefunden: ");
        println(discReq.channelId());
#endif
        KnxIpDisconnectResponse discRes(0x00, E_CONNECTION_ID);
        _platform.sendBytesUniCast(discReq.hpaiCtrl().ipAddress(), discReq.hpaiCtrl().ipPortNumber(), discRes.data(), discRes.totalLength());
        return;
    }


    KnxIpDisconnectResponse discRes(tun->ChannelId, E_NO_ERROR);
    _platform.sendBytesUniCast(discReq.hpaiCtrl().ipAddress(), discReq.hpaiCtrl().ipPortNumber(), discRes.data(), discRes.totalLength());
    tun->Reset();
}

void IpDataLinkLayer::loopHandleDescriptionRequest(uint8_t* buffer, uint16_t length)
{
    KnxIpDescriptionRequest descReq(buffer, length);
    KnxIpDescriptionResponse descRes(_ipParameters, _deviceObject);
    _platform.sendBytesUniCast(descReq.hpaiCtrl().ipAddress(), descReq.hpaiCtrl().ipPortNumber(), descRes.data(), descRes.totalLength());
}

void IpDataLinkLayer::loopHandleDeviceConfigurationRequest(uint8_t* buffer, uint16_t length)
{
    KnxIpConfigRequest confReq(buffer, length);
    
    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].ChannelId == confReq.connectionHeader().channelId())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
        print("Channel ID nicht gefunden: ");
        println(confReq.connectionHeader().channelId());
        KnxIpStateResponse stateRes(0x00, E_CONNECTION_ID);
        _platform.sendBytesUniCast(0, 0, stateRes.data(), stateRes.totalLength());
        return;
    }

    KnxIpTunnelingAck tunnAck;
    tunnAck.serviceTypeIdentifier(DeviceConfigurationAck);
    tunnAck.connectionHeader().length(4);
    tunnAck.connectionHeader().channelId(tun->ChannelId);
    tunnAck.connectionHeader().sequenceCounter(confReq.connectionHeader().sequenceCounter());
    tunnAck.connectionHeader().status(E_NO_ERROR);
    _platform.sendBytesUniCast(tun->IpAddress, tun->PortData, tunnAck.data(), tunnAck.totalLength());

    tun->lastHeartbeat = millis();
    _cemiServer->frameReceived(confReq.frame());
}

void IpDataLinkLayer::loopHandleTunnelingRequest(uint8_t* buffer, uint16_t length)
{
    KnxIpTunnelingRequest tunnReq(buffer, length);

    KnxIpTunnelConnection *tun = nullptr;
    for(int i = 0; i < KNX_TUNNELING; i++)
    {
        if(tunnels[i].ChannelId == tunnReq.connectionHeader().channelId())
        {
            tun = &tunnels[i];
            break;
        }
    }

    if(tun == nullptr)
    {
#ifdef KNX_LOG_TUNNELING
        print("Channel ID nicht gefunden: ");
        println(tunnReq.connectionHeader().channelId());
#endif
        KnxIpStateResponse stateRes(0x00, E_CONNECTION_ID);
        _platform.sendBytesUniCast(0, 0, stateRes.data(), stateRes.totalLength());
        return;
    }

    uint8_t sequence = tunnReq.connectionHeader().sequenceCounter();
    if(sequence == tun->SequenceCounter_R)
    {
#ifdef KNX_LOG_TUNNELING
        print("Received SequenceCounter again: ");
        println(tunnReq.connectionHeader().sequenceCounter());
#endif
        //we already got this one
        //so just ack it
        KnxIpTunnelingAck tunnAck;
        tunnAck.connectionHeader().length(4);
        tunnAck.connectionHeader().channelId(tun->ChannelId);
        tunnAck.connectionHeader().sequenceCounter(tunnReq.connectionHeader().sequenceCounter());
        tunnAck.connectionHeader().status(E_NO_ERROR);
        _platform.sendBytesUniCast(tun->IpAddress, tun->PortData, tunnAck.data(), tunnAck.totalLength());
        return;
    } else if((uint8_t)(sequence - 1) != tun->SequenceCounter_R) {
#ifdef KNX_LOG_TUNNELING
        print("Wrong SequenceCounter: got ");
        print(tunnReq.connectionHeader().sequenceCounter());
        print(" expected ");
        println((uint8_t)(tun->SequenceCounter_R + 1));
#endif
        //Dont handle it
        return;
    }
    
    KnxIpTunnelingAck tunnAck;
    tunnAck.connectionHeader().length(4);
    tunnAck.connectionHeader().channelId(tun->ChannelId);
    tunnAck.connectionHeader().sequenceCounter(tunnReq.connectionHeader().sequenceCounter());
    tunnAck.connectionHeader().status(E_NO_ERROR);
    _platform.sendBytesUniCast(tun->IpAddress, tun->PortData, tunnAck.data(), tunnAck.totalLength());

    tun->SequenceCounter_R = tunnReq.connectionHeader().sequenceCounter();

    if(tunnReq.frame().sourceAddress() == 0)
        tunnReq.frame().sourceAddress(tun->IndividualAddress);

    _cemiServer->frameReceived(tunnReq.frame());
}
#endif

void IpDataLinkLayer::enabled(bool value)
{
//    _print("own address: ");
//    _println(_deviceObject.individualAddress());
    if (value && !_enabled)
    {
        _platform.setupMultiCast(_ipParameters.propertyValue<uint32_t>(PID_ROUTING_MULTICAST_ADDRESS), KNXIP_MULTICAST_PORT);
        _enabled = true;
        return;
    }

    if(!value && _enabled)
    {
        _platform.closeMultiCast();
        _enabled = false;
        return;
    }
}

bool IpDataLinkLayer::enabled() const
{
    return _enabled;
}

DptMedium IpDataLinkLayer::mediumType() const
{
    return DptMedium::KNX_IP;
}

bool IpDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    if (!_enabled)
        return false;

    return _platform.sendBytesMultiCast(bytes, length);
}

bool IpDataLinkLayer::isSendLimitReached()
{
    uint32_t curTime = millis() / 100;

    // check if the countbuffer must be adjusted
    if(_frameCountTimeBase >= curTime)
    {
        uint32_t timeBaseDiff = _frameCountTimeBase - curTime;
        if(timeBaseDiff > 10)
            timeBaseDiff = 10;
        for(int i = 0; i < timeBaseDiff ; i++)
        {
            _frameCountBase++;
            _frameCountBase = _frameCountBase % 10;
            _frameCount[_frameCountBase] = 0;
        }
        _frameCountTimeBase = curTime;
    }
    else // _frameCountTimeBase < curTime => millis overflow, reset
    {
        for(int i = 0; i < 10 ; i++)
            _frameCount[i] = 0;
        _frameCountBase = 0;
        _frameCountTimeBase = curTime;
    }

    //check if we are over the limit
    uint16_t sum = 0;
    for(int i = 0; i < 10 ; i++)
        sum += _frameCount[i];
    if(sum > 50)
    {
        println("Dropping packet due to 50p/s limit");
        return true;   // drop packet
    }
    else
    {
        _frameCount[_frameCountBase]++;
        //print("sent packages in last 1000ms: ");
        //print(sum);
        //print(" curTime: ");
        //println(curTime);
        return false;
    }
}
#endif
