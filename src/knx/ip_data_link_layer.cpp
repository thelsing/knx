#include "ip_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

#define ROUTING_INDICATION 0x0530

#define KNXIP_MULTICAST_PORT 3671
#define MIN_LEN_CEMI 10

IpDataLinkLayer::IpDataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, IpParameterObject& ipParam, 
    NetworkLayer& layer, Platform& platform) : DataLinkLayer(devObj, addrTab, layer, platform), _ipParameters(ipParam)
{
}

bool IpDataLinkLayer::sendFrame(CemiFrame& frame)
{
    uint16_t length = frame.totalLenght() + KNXIP_HEADER_LEN;
    uint8_t* buffer = new uint8_t[length];
    buffer[0] = KNXIP_HEADER_LEN;
    buffer[1] = KNXIP_PROTOCOL_VERSION;
    pushWord(ROUTING_INDICATION, buffer + 2);
    pushWord(length, buffer + 4);

    memcpy(buffer + KNXIP_HEADER_LEN, frameData(frame), frame.totalLenght());
    
    bool success = sendBytes(buffer, length);
    // only send 50 packet per second: see KNX 3.2.6 p.6
    _platform.mdelay(20);
    dataConReceived(frame, success);
    delete[] buffer;
    return success;
}

void IpDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    uint8_t buffer[512];
    int len = _platform.readBytes(buffer, 512);
    if (len <= 0)
        return;

    if (len < KNXIP_HEADER_LEN)
        return;
    
    if (buffer[0] != KNXIP_HEADER_LEN 
        || buffer[1] != KNXIP_PROTOCOL_VERSION)
        return;

    uint16_t code;
    popWord(code, buffer + 2);
    if (code != ROUTING_INDICATION) // only routing indication for now
        return;
    
    if (len < MIN_LEN_CEMI)
        return;

    //TODO: Check correct length (additions Info + apdu length)
    CemiFrame frame(buffer + KNXIP_HEADER_LEN, len - KNXIP_HEADER_LEN);
    frameRecieved(frame);
}

void IpDataLinkLayer::enabled(bool value)
{
//    _print("own address: ");
//    _println(_deviceObject.induvidualAddress());
    if (value && !_enabled)
    {
        _platform.setupMultiCast(_ipParameters.multicastAddress(), KNXIP_MULTICAST_PORT);
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

    return _platform.sendBytes(bytes, length);
}
