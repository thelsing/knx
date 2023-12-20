#pragma once

#include "cemi_frame.h"
#include "config.h"
#ifdef USE_IP

#define LEN_KNXIP_HEADER 0x6

enum KnxIpVersion
{
    KnxIp1_0 = 0x10
};

enum KnxIpServiceType
{
    SearchRequest = 0x201,
    SearchResponse = 0x202,
    DescriptionRequest = 0x203,
    DescriptionResponse = 0x204,
    ConnectRequest = 0x205,
    ConnectResponse = 0x206,
    ConnectionStateRequest = 0x207,
    ConnectionStateResponse = 0x208,
    DisconnectRequest = 0x209,
    DisconnectResponse = 0x20A,
    SearchRequestExt = 0x20B,
    SearchResponseExt = 0x20C,
    DeviceConfigurationRequest = 0x310,
    DeviceConfigurationAck = 0x311,
    TunnelingRequest = 0x420,
    TunnelingAck = 0x421,
    RoutingIndication = 0x530,
    RoutingLostMessage = 0x531,
};

class KnxIpFrame
{
  public:
    KnxIpFrame(uint8_t* data, uint16_t length);
    KnxIpFrame(uint16_t totalLength);
    virtual ~KnxIpFrame();
    uint8_t headerLength() const;
    void headerLength(uint8_t length);
    KnxIpVersion protocolVersion() const;
    void protocolVersion(KnxIpVersion version);
    uint16_t serviceTypeIdentifier() const;
    void serviceTypeIdentifier(uint16_t identifier);
    uint16_t totalLength() const;
    void totalLength(uint16_t length);
    uint8_t* data();

  protected:
    bool _freeData = false;
    uint8_t* _data = 0;
    uint16_t _dataLength;
};
#endif
