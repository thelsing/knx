#pragma once

#include "config.h"

#include <stdint.h>
#include "device_object.h"
#include "knx_types.h"
#include "network_layer_entity.h"
#ifdef KNX_TUNNELING
    #include "ip_parameter_object.h"
#endif
#include "cemi_server.h"
#include "bau.h"

class Platform;

typedef void (*ActivityCallback)(uint8_t info);

class DataLinkLayerCallbacks
{
    protected:
        ActivityCallback _activityCallback = nullptr;
    public:
        virtual ~DataLinkLayerCallbacks() = default;
        virtual void activity(uint8_t info);
        virtual void setActivityCallback(ActivityCallback activityCallback);
};

class DataLinkLayer
{
    public:
        DataLinkLayer(DeviceObject& devObj, NetworkLayerEntity& netLayerEntity,
                      Platform& platform);

#ifdef USE_CEMI_SERVER
        // from tunnel
        void cemiServer(CemiServer& cemiServer);
        void dataRequestFromTunnel(CemiFrame& frame);
#ifdef KNX_TUNNELING
        virtual void dataRequestToTunnel(CemiFrame& frame);
        virtual void dataConfirmationToTunnel(CemiFrame& frame);
        virtual void dataIndicationToTunnel(CemiFrame& frame);
        virtual bool isTunnelAddress(uint16_t addr);
        void ipParameterObject(IpParameterObject* object);
#endif
#endif

        // from network layer
        void dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, uint16_t sourceAddr, FrameFormat format,
                         Priority priority, NPDU& npdu);
        void systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu, uint16_t sourceAddr);
        virtual void loop() = 0;
        virtual void enabled(bool value) = 0;
        virtual bool enabled() const = 0;
        virtual DptMedium mediumType() const = 0;

    protected:
        void frameReceived(CemiFrame& frame);
        void dataConReceived(CemiFrame& frame, bool success);
        bool sendTelegram(NPDU& npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, uint16_t sourceAddr, FrameFormat format, Priority priority, SystemBroadcast systemBroadcast, bool doNotRepeat = false);
        virtual bool sendFrame(CemiFrame& frame) = 0;
        uint8_t* frameData(CemiFrame& frame);
        DeviceObject& _deviceObject;
        NetworkLayerEntity& _networkLayerEntity;
        Platform& _platform;
#ifdef USE_CEMI_SERVER
        CemiServer* _cemiServer;
#endif
#ifdef KNX_ACTIVITYCALLBACK
        uint8_t _netIndex = 0;
#endif
#ifdef KNX_TUNNELING
        bool isTunnelingPA(uint16_t pa);
        bool isRoutedPA(uint16_t pa);
        IpParameterObject* _ipParameters;
#endif
};
