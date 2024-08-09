#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "npdu.h"
#include "transport_layer.h"
#include "network_layer_entity.h"
#include "network_layer.h"

class DeviceObject;
class RouterObject;

class NetworkLayerCoupler : public NetworkLayer
{
    friend class NetworkLayerEntity;

  public:
    NetworkLayerCoupler(DeviceObject& deviceObj, TransportLayer& layer);

    NetworkLayerEntity& getPrimaryInterface();
    NetworkLayerEntity& getSecondaryInterface();

    bool isRoutedIndividualAddress(uint16_t individualAddress, uint8_t srcIfIndex);

    bool isRoutedGroupAddress(uint16_t groupAddress, uint8_t sourceInterfaceIndex);

    void rtObjPrimary(RouterObject& rtObjPrimary); // Coupler model 2.0
    void rtObjSecondary(RouterObject& rtObjSecondary); // Coupler model 2.0
    void rtObj(RouterObject& rtObj); // Coupler model 1.x

    // from transport layer
    void dataIndividualRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    void dataGroupRequest(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) override;
    void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu) override;

  private:
    enum CouplerType
    {
        LineCoupler,
        BackboneCoupler,
        TP1Bridge,
        TP1Repeater
    };

    static constexpr uint8_t kPrimaryIfIndex = 0;
    static constexpr uint8_t kSecondaryIfIndex = 1;
    static constexpr uint8_t kLocalIfIndex = 99;

    // from entities
    void dataIndication(AckType ack, AddressType addType, uint16_t destination, FrameFormat format, NPDU& npdu,
                        Priority priority, uint16_t source, uint8_t srcIfIdx) override;
    void dataConfirm(AckType ack, AddressType addrType, uint16_t destination, FrameFormat format, Priority priority,
                     uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx) override;
    void broadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                             Priority priority, uint16_t source, uint8_t srcIfIdx) override;
    void broadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx) override;
    void systemBroadcastIndication(AckType ack, FrameFormat format, NPDU& npdu,
                                   Priority priority, uint16_t source, uint8_t srcIfIdx) override;
    void systemBroadcastConfirm(AckType ack, FrameFormat format, Priority priority, uint16_t source, NPDU& npdu, bool status, uint8_t srcIfIdx) override;

    void routeDataIndividual(AckType ack, uint16_t destination, NPDU& npdu, Priority priority, uint16_t source, uint8_t srcIfIndex);
    void sendMsgHopCount(AckType ack, AddressType addrType, uint16_t destination, NPDU& npdu, Priority priority,
                      SystemBroadcast broadcastType, uint8_t sourceInterfaceIndex, uint16_t source);

    void evaluateCouplerType();
    bool isGroupAddressInFilterTable(uint16_t groupAddress);
#ifdef KNX_TUNNELING
    bool isTunnelAddress(uint16_t destination);
#endif

    // Support a maximum of two physical interfaces for couplers
    NetworkLayerEntity _netLayerEntities[2];

    RouterObject* _rtObjPrimary {nullptr};
    RouterObject* _rtObjSecondary {nullptr};

    CouplerType _couplerType;
    uint16_t _currentAddress;
};
