#pragma once

#include "application_layer.h"
#include <stdint.h>
#include "knx_types.h"
#include "apdu.h"
#include "bits.h"
#include "simple_map.h"

class DeviceObject;
class SecurityInterfaceObject;
class AssociationTableObject;
class AddressTableObject;
class BusAccessUnit;
/**
 * This is an implementation of the application layer as specified in @cite knx:3/5/1.
 * It provides methods for the BusAccessUnit to do different things and translates this 
 * call to an APDU and calls the correct method of the TransportLayer. 
 * It also takes calls from TransportLayer, decodes the submitted APDU and calls the coresponding
 * methods of the BusAccessUnit class.
 */
class SecureApplicationLayer :  public ApplicationLayer
{
  public:
    /**
     * The constructor.
     * @param assocTable The AssociationTable is used to translate between asap (i.e. group objects) and group addresses.
     * @param bau methods are called here depending of the content of the APDU
     */
    SecureApplicationLayer(DeviceObject& deviceObj, SecurityInterfaceObject& secIfObj, BusAccessUnit& bau);

    void groupAddressTable(AddressTableObject& addrTable);

    // from transport layer
     void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu) override;
     void dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap,
                                  APDU& apdu, bool status) override;
     void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu) override;
     void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status) override;
     void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu) override;
     void dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status) override;
     void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu) override;
     void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status) override;
     void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu) override;
     void dataConnectedConfirm(uint16_t tsap) override;

    void loop();

  protected:
    // to transport layer
     void dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl& secCtrl) override;
     void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl) override;
     void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl) override;
     void dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu, const SecurityControl& secCtrl) override;
     void dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu, const SecurityControl& secCtrl) override; // apdu must be valid until it was confirmed

  private:
    enum class AddrType : uint8_t
    {
        group,
        individual,
        unknown
    };

    struct Addr
    {
        Addr() = default;
        Addr(uint8_t addr) : addr{addr} {}

        uint16_t addr;
        AddrType addrType{AddrType::unknown};

        bool operator ==(const Addr &cmpAddr) const
        {
            if ((cmpAddr.addrType == AddrType::unknown) || (addrType == AddrType::unknown))
            {
                println("Unknown address type detected!");
                return false;
            }
            return (cmpAddr.addr == addr) && (cmpAddr.addrType == addrType);
        }
    };

    struct GrpAddr : Addr
    {
        GrpAddr() {addrType = AddrType::group;}
        GrpAddr(uint8_t addr) : Addr{addr} {addrType = AddrType::group;}
    };

    struct IndAddr : Addr
    {
        IndAddr() { addrType = AddrType::individual; }
        IndAddr(uint8_t addr) : Addr{addr} { addrType = AddrType::individual; }
    };

    uint32_t calcAuthOnlyMac(uint8_t* apdu, uint8_t apduLength, const uint8_t *key, uint8_t* iv, uint8_t* ctr0);
    uint32_t calcConfAuthMac(uint8_t* associatedData, uint16_t associatedDataLength, uint8_t* apdu, uint8_t apduLength, const uint8_t* key, uint8_t* iv);

    void block0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t extFrameFormat, uint8_t tpci, uint8_t apci, uint8_t payloadLength);
    void blockCtr0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr);

    const uint8_t* securityKey(uint16_t addr, bool isGroupAddress);

    uint16_t groupAddressIndex(uint16_t groupAddr);     // returns 1-based index of address in group address table
    uint16_t groupObjectIndex(uint16_t groupAddrIndex); // returns 1-based index of object in association table

    uint8_t groupObjectSecurity(uint16_t groupObjectIndex);

    uint64_t nextSequenceNumber(bool toolAccess);
    void updateSequenceNumber(bool toolAccess, uint64_t seqNum);

    uint64_t lastValidSequenceNumber(bool toolAcces, uint16_t srcAddr);
    void updateLastValidSequence(bool toolAccess, uint16_t remoteAddr, uint64_t seqNo);

    uint64_t getRandomNumber();

    bool isSyncService(APDU& secureAsdu);

    void sendSyncRequest(uint16_t dstAddr, bool dstAddrIsGroupAddr, const SecurityControl &secCtrl, bool systemBcast);
    void sendSyncResponse(uint16_t dstAddr, bool dstAddrIsGroupAddr, const SecurityControl &secCtrl, uint64_t remoteNextSeqNum, bool systemBcast);
    void receivedSyncRequest(uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, const SecurityControl &secCtrl, uint8_t* seq, uint64_t challenge, bool systemBcast);
    void receivedSyncResponse(uint16_t remoteAddr, const SecurityControl &secCtrl, uint8_t* plainApdu);

    bool decrypt(uint8_t* plainApdu, uint16_t plainapduLength, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci, uint8_t* secureAsdu, SecurityControl &secCtrl, bool systemBcast);
    bool secure(uint8_t* buffer, uint16_t service, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci, uint8_t* apdu, uint16_t apduLength, const SecurityControl &secCtrl, bool systemBcast);

    bool decodeSecureApdu(APDU& secureApdu, APDU& plainApdu, SecurityControl &secCtrl);
    bool createSecureApdu(APDU& plainApdu, APDU& secureApdu, const SecurityControl &secCtrl);

    void encryptAesCbc(uint8_t* buffer, uint16_t bufLen, const uint8_t* iv, const uint8_t* key);
    void xcryptAesCtr(uint8_t* buffer, uint16_t bufLen, const uint8_t* iv, const uint8_t* key);

    bool _syncReqBroadcastIncoming{false};
    bool _syncReqBroadcastOutgoing{false};
    uint32_t _lastSyncRes;

    Map<Addr, uint64_t, 1> _pendingOutgoingSyncRequests; // Store challenges for outgoing sync requests
    Map<Addr, uint64_t, 1> _pendingIncomingSyncRequests; // Store challenges for incoming sync requests

    uint64_t _sequenceNumberToolAccess = 50;
    uint64_t _sequenceNumber = 0;

    uint64_t _lastValidSequenceNumberTool = 0;
    uint64_t _lastValidSequenceNumber = 0;

    SecurityInterfaceObject& _secIfObj;
    DeviceObject& _deviceObj;
    AddressTableObject* _addrTab = nullptr;
};
