#pragma once

#include <stdint.h>
#include "../datalink_layer/data_link_layer.h"
#include "ip_parameter_object.h"
#include "knx_ip_tunnel_connection.h"
#include "service_families.h"
#include "knx_ip_frame.h"

namespace Knx
{
    class IpDataLinkLayer : public DataLinkLayer
    {
            using DataLinkLayer::_deviceObject;

        public:
            IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam, NetworkLayerEntity& netLayerEntity,
                            Platform& platform, DataLinkLayerCallbacks* dllcb = nullptr);

            void loop();
            void enabled(bool value);
            bool enabled() const;
            DptMedium mediumType() const override;
#ifdef KNX_TUNNELING
            void dataRequestToTunnel(CemiFrame& frame) override;
            void dataConfirmationToTunnel(CemiFrame& frame) override;
            void dataIndicationToTunnel(CemiFrame& frame) override;
            bool isTunnelAddress(uint16_t addr) override;
#endif
            bool isSentToTunnel(uint16_t address, bool isGrpAddr);

        private:
            bool _enabled = false;
            uint8_t _frameCount[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            uint8_t _frameCountBase = 0;
            uint32_t _frameCountTimeBase = 0;
            bool sendFrame(CemiFrame& frame);
#ifdef KNX_TUNNELING
            void sendFrameToTunnel(KnxIpTunnelConnection* tunnel, CemiFrame& frame);
            void loopHandleConnectRequest(uint8_t* buffer, uint16_t length, uint32_t& src_addr, uint16_t& src_port);
            void loopHandleConnectionStateRequest(uint8_t* buffer, uint16_t length);
            void loopHandleDisconnectRequest(uint8_t* buffer, uint16_t length);
            void loopHandleTunnelingRequest(uint8_t* buffer, uint16_t length);
            void loopHandleDeviceConfigurationRequest(uint8_t* buffer, uint16_t length);
#endif
            void loopHandleDescriptionRequest(uint8_t* buffer, uint16_t length);
            void loopHandleSearchRequestExtended(uint8_t* buffer, uint16_t length);
            bool sendMulicast(KnxIpFrame& ipFrame);
            bool sendUnicast(uint32_t addr, uint16_t port, KnxIpFrame& ipFrame);
            bool isSendLimitReached();
            IpParameterObject& _ipParameters;
            DataLinkLayerCallbacks* _dllcb;
#ifdef KNX_TUNNELING
            KnxIpTunnelConnection tunnels[KNX_TUNNELING];
            uint8_t _lastChannelId = 0;
#endif
    };
}