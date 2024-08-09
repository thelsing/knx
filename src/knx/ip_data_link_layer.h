#pragma once

#include "config.h"
#ifdef USE_IP

#include <stdint.h>
#include "data_link_layer.h"
#include "ip_parameter_object.h"
#include "knx_ip_tunnel_connection.h"
#include "service_families.h"

class IpDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;

  public:
    IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam, NetworkLayerEntity& netLayerEntity,
                    Platform& platform, BusAccessUnit& busAccessUnit, DataLinkLayerCallbacks* dllcb = nullptr);

    void loop();
    void enabled(bool value);
    bool enabled() const;
    DptMedium mediumType() const override;
#ifdef KNX_TUNNELING
    void dataRequestToTunnel(CemiFrame& frame) override;
    void dataConfirmationToTunnel(CemiFrame& frame) override;
    void dataIndicationToTunnel(CemiFrame& frame) override;
    bool isTunnelAddress(uint16_t addr) override;
    bool isSentToTunnel(uint16_t address, bool isGrpAddr);
#endif

  private:
    bool _enabled = false;
    uint8_t _frameCount[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t _frameCountBase = 0;
    uint32_t _frameCountTimeBase = 0;
    bool sendFrame(CemiFrame& frame);
#ifdef KNX_TUNNELING
    void sendFrameToTunnel(KnxIpTunnelConnection *tunnel, CemiFrame& frame);
    void loopHandleConnectRequest(uint8_t* buffer, uint16_t length, uint32_t& src_addr, uint16_t& src_port);
    void loopHandleConnectionStateRequest(uint8_t* buffer, uint16_t length);
    void loopHandleDisconnectRequest(uint8_t* buffer, uint16_t length);
    void loopHandleDescriptionRequest(uint8_t* buffer, uint16_t length);
    void loopHandleDeviceConfigurationRequest(uint8_t* buffer, uint16_t length);
    void loopHandleTunnelingRequest(uint8_t* buffer, uint16_t length);
#endif
#if KNX_SERVICE_FAMILY_CORE >= 2
    void loopHandleSearchRequestExtended(uint8_t* buffer, uint16_t length);
#endif
    bool sendBytes(uint8_t* buffer, uint16_t length);
    bool isSendLimitReached();

    IpParameterObject& _ipParameters;
    DataLinkLayerCallbacks* _dllcb;
#ifdef KNX_TUNNELING
    KnxIpTunnelConnection tunnels[KNX_TUNNELING];
    uint8_t _lastChannelId = 1;
#endif
};
#endif