#include "ip_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "knx_ip_routing_indication.h"
#include "knx_ip_search_request.h"
#include "knx_ip_search_response.h"

#include <stdio.h>
#include <string.h>

#ifdef USE_IP

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

#define MIN_LEN_CEMI 10

IpDataLinkLayer::IpDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, IpParameterObject& ipParam, 
    NetworkLayer& layer, Platform& platform) : DataLinkLayer(devObj, addrTab, layer, platform), _ipParameters(ipParam)
{
}

bool IpDataLinkLayer::sendFrame(CemiFrame& frame)
{
    KnxIpRoutingIndication packet(frame);
    
    bool success = sendBytes(packet.data(), packet.totalLength());
    // only send 50 packet per second: see KNX 3.2.6 p.6
    delay(20);
    dataConReceived(frame, success);
    return success;
}

void IpDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    uint8_t buffer[512];
    int len = _platform.readBytesMultiCast(buffer, 512);
    if (len <= 0)
        return;

    if (len < KNXIP_HEADER_LEN)
        return;
    
    if (buffer[0] != KNXIP_HEADER_LEN 
        || buffer[1] != KNXIP_PROTOCOL_VERSION)
        return;

    uint16_t code;
    popWord(code, buffer + 2);
    switch ((KnxIpServiceType)code)
    {
        case RoutingIndication:
        {
            KnxIpRoutingIndication routingIndication(buffer, len);
            frameRecieved(routingIndication.frame());
            break;
        }
        case SearchRequest:
        {
            KnxIpSearchRequest searchRequest(buffer, len);
            KnxIpSearchResponse searchResponse(_ipParameters, _deviceObject);

            auto hpai = searchRequest.hpai();
            _platform.sendBytesUniCast(hpai.ipAddress(), hpai.ipPortNumber(), searchResponse.data(), searchResponse.totalLength());
            break;
        }
        default:
            print("Unhandled service identifier: ");
            println(code, HEX);
    }
}

void IpDataLinkLayer::enabled(bool value)
{
//    _print("own address: ");
//    _println(_deviceObject.induvidualAddress());
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


bool IpDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    if (!_enabled)
        return false;

    return _platform.sendBytesMultiCast(bytes, length);
}
#endif