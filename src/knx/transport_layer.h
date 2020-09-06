#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "tpdu.h"
#include "address_table_object.h"
#include "cemi_frame.h"

class ApplicationLayer;
class APDU;
class NetworkLayer;
class Platform;

enum StateType { Closed, OpenIdle, OpenWait, Connecting };

class TransportLayer
{
public:
    TransportLayer(ApplicationLayer& layer);
    void networkLayer(NetworkLayer& layer);
    void groupAddressTable(AddressTableObject& addrTable);

#pragma region from network layer
    void dataIndividualIndication(uint16_t destination, HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu);
    void dataIndividualConfirm(AckType ack, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu, bool status);
    void dataGroupIndication(uint16_t destination, HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu);
    void dataGroupConfirm(AckType ack, uint16_t source, uint16_t destination, HopCountType hopType, Priority priority, TPDU& tpdu, bool status);
    void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu);
    void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, TPDU& tpdu, bool status);
    void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, TPDU& tpdu);
    void dataSystemBroadcastConfirm(AckType ack, HopCountType hopType, TPDU& tpdu, Priority priority, bool status);
#pragma endregion
    
#pragma region from application layer
    /**
     * Request to send an APDU that via multicast. See 3.2 of @cite knx:3/3/4. 
     * See also ApplicationLayer::dataGroupConfirm and ApplicationLayer::dataGroupIndication. 
     * This method is called by the ApplicationLayer.
     * 
     * @param tsap used the find the correct GroupObject with the help of the AssociationTableObject. 
     *        See 3.1.1 of @cite knx:3/3/7
     *        
     * @param apdu The submitted APDU.
     * 
     * @param priority The ::Priority of the request.
     * 
     * @param hopType Should routing be endless or should the NetworkLayer::hopCount be used? See also ::HopCountType.
     * 
     * @param ack Did we want a DataLinkLayer acknowledgement? See ::AckType.
     */
    void dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu);
    void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu);
    void dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu);
    
    void connectRequest(uint16_t destination, Priority priority);
    void disconnectRequest(uint16_t tsap, Priority priority);
    // apdu must be valid until it was confirmed
    void dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu);

    uint8_t getTpciSeqNum();
    uint16_t getConnectionAddress();
#pragma endregion

#pragma region other
    void connectionTimeoutIndication();
    void ackTimeoutIndication();
    void loop();
#pragma endregion
    
private:
#pragma region States
    Priority _savedPriority = LowPriority;
    CemiFrame _savedFrame;
    Priority _savedPriorityConnecting;
    CemiFrame _savedFrameConnecting;
    uint16_t _savedTsapConnecting;
    bool _savedConnectingValid = false;
    enum StateEvent
    {
        E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14,
        E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27
    };
    StateType _currentState = Closed;
    void sendControlTelegram(TpduType pduType, uint8_t seqNo);
    void A0();
    void A1(uint16_t source);
    void A2(uint16_t source, Priority priority, APDU& apdu);
    void A3(uint16_t source, Priority priority, TPDU& recTpdu);
    void A4(uint16_t source, Priority priority, TPDU& recTpdu);
    void A5(uint16_t source);
    void A6(uint16_t source);
    void A7(Priority priority, APDU& apdu);
    void A8();
    void A9();
    void A10(uint16_t source);
    void A11(uint16_t tsap, Priority priority, APDU& apdu);
    void A12(uint16_t destination, Priority priority);
    void A13(uint16_t destination);
    void A14(uint16_t destination, Priority priority);
    void A15(Priority priority, uint16_t tsap);
    void enableConnectionTimeout();
    void disableConnectionTimeout();
    void enableAckTimeout();
    void disableAckTimeout();
    uint16_t _connectionAddress = 0;
    uint8_t _seqNoSend = 0;
    uint8_t _seqNoRecv = 0;
    bool _connectionTimeoutEnabled = false;
    uint32_t _connectionTimeoutStartMillis = 0;
    uint16_t _connectionTimeoutMillis = 6000;
    bool _ackTimeoutEnabled = false;
    uint32_t _ackTimeoutStartMillis = 0;
    uint16_t _ackTimeoutMillis = 3000;
    uint8_t _repCount = 0;
    uint8_t _maxRepCount = 3;
#pragma endregion
    ApplicationLayer& _applicationLayer;
    AddressTableObject* _groupAddressTable;
    NetworkLayer* _networkLayer;
};
