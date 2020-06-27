#pragma once

#include "application_layer.h"
#include <stdint.h>
#include "knx_types.h"
#include "apdu.h"
#include "bits.h"

class DeviceObject;
class SecurityInterfaceObject;
class AssociationTableObject;
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
    SecureApplicationLayer(DeviceObject& deviceObj, SecurityInterfaceObject& secIfObj, AssociationTableObject& assocTable, BusAccessUnit& bau);

    void setSecurityMode(bool enabled);
    bool isSecurityModeEnabled();
    void clearFailureLog();
    void getFailureCounters(uint8_t* data);
    uint8_t getFromFailureLogByIndex(uint8_t index, uint8_t* data, uint8_t maxDataLen);

    // from transport layer
    virtual void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu) override;
    virtual void dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap,
                                  APDU& apdu, bool status) override;
    virtual void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu) override;
    virtual void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status) override;
    virtual void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu) override;
    virtual void dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status) override;
    virtual void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu) override;
    virtual void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status) override;
    virtual void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu) override;
    virtual void dataConnectedConfirm(uint16_t tsap) override;

    void loop();

  protected:
    // to transport layer
    virtual void dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl& secCtrl) override;
    virtual void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl) override;
    virtual void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl) override;
    virtual void dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu, const SecurityControl& secCtrl) override;
    virtual void dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu, const SecurityControl& secCtrl) override; // apdu must be valid until it was confirmed

  private:

    template <typename K, typename V, int SIZE>
    class Map
    {
    public:
        Map()
        {
            static_assert (SIZE <= 64, "Map is too big! Max. 64 elements.");
        }

        void clear()
        {
            _validEntries = 0;
        }

        bool empty()
        {
            return (_validEntries == 0);
        }

        uint8_t size()
        {
            uint8_t size = 0;

            for (uint8_t i = 0; i < SIZE; i++)
            {
                size += (((_validEntries >> i) & 0x01) == 0x01) ? 1 : 0;
            }

            return size;
        }

        bool insert(K key, V value)
        {
            uint8_t index = getNextFreeIndex();
            if (index != noFreeEntryFoundIndex)
            {
                keys[index] = key;
                values[index] = value;

                _validEntries |= 1 << index;
                return true;
            }

            // No free space
            return false;
        }

        bool insertOrAssign(K key, V value)
        {
            // Try to find the key
            for (uint8_t i = 0; i < SIZE; i++)
            {
                // Check if this array slot is occupied
                if ((_validEntries >> i) & 0x01)
                {
                    // Key found?
                    if (keys[i] == key)
                    {
                        values[i] = value;
                        return true;
                    }
                }
            }

            // Key does not exist, add it if enough space
            return insert(key, value);
        }

        bool erase(K key)
        {
            for (uint8_t i = 0; i < SIZE; i++)
            {
                if ((_validEntries >> i) & 0x01)
                {
                    if (keys[i] == key)
                    {
                        _validEntries &= ~(1 << i);
                        return true;
                    }
                }
            }
            return false;
        }

        V* get(K key)
        {
            // Try to find the key
            for (uint8_t i = 0; i < SIZE; i++)
            {
                // Check if this array slot is occupied
                if ((_validEntries >> i) & 0x01)
                {
                    // Key found?
                    if (keys[i] == key)
                    {
                        return &values[i];
                    }
                }
            }
            return nullptr;
        }

    private:
        uint8_t getNextFreeIndex()
        {
            for (uint8_t i = 0; i < SIZE; i++)
            {
                if (((_validEntries >> i) & 0x01) == 0)
                {
                    return i;
                }
            }

            return noFreeEntryFoundIndex;
        }

        uint64_t _validEntries{0};
        K keys[SIZE];
        V values[SIZE];
        static constexpr uint8_t noFreeEntryFoundIndex = 255;
    };

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

    void sixBytesFromUInt64(uint64_t num, uint8_t* toByteArray);
    uint64_t sixBytesToUInt64(uint8_t* data);

    const uint8_t *toolKey(uint16_t devAddr);
    const uint8_t* securityKey(uint16_t addr, bool isGroupAddress);

    uint16_t indAddressIndex(uint16_t indAddr);         // returns 1-based index of address in security IA table
    uint16_t groupAddressIndex(uint16_t groupAddr);     // returns 1-based index of address in group address table
    uint16_t groupObjectIndex(uint16_t groupAddrIndex); // returns 1-based index of object in association table
    const uint8_t* p2pKey(uint16_t addressIndex);       // returns p2p key for IA index
    const uint8_t* groupKey(uint16_t addressIndex);     // returns group key for group address index

    uint8_t groupObjectSecurity(uint16_t groupObjectIndex);

    uint64_t nextSequenceNumber(bool toolAccess);
    void updateSequenceNumber(bool toolAccess, uint64_t seqNum);

    uint64_t lastValidSequenceNumber(bool toolAcces, uint16_t srcAddr);
    void updateLastValidSequence(bool toolAccess, uint16_t remoteAddr, uint64_t seqNo);

    uint64_t getRandomNumber();

    bool isSyncService(APDU& secureAsdu);

    void sendSyncRequest(uint16_t dstAddr, bool dstAddrIsGroupAddr, const SecurityControl &secCtrl);
    void sendSyncResponse(uint16_t dstAddr, bool dstAddrIsGroupAddr, const SecurityControl &secCtrl, uint64_t remoteNextSeqNum);
    void receivedSyncRequest(uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, const SecurityControl &secCtrl, uint8_t* seq, uint64_t challenge);
    void receivedSyncResponse(uint16_t remoteAddr, const SecurityControl &secCtrl, uint8_t* plainApdu);

    bool decrypt(uint8_t* plainApdu, uint16_t plainapduLength, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci, uint8_t* secureAsdu, SecurityControl &secCtrl);
    bool secure(uint8_t* buffer, uint16_t service, uint16_t srcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t tpci, uint8_t* apdu, uint16_t apduLength, const SecurityControl &secCtrl);

    bool decodeSecureApdu(APDU& secureApdu, APDU& plainApdu, SecurityControl &secCtrl);
    bool createSecureApdu(APDU& plainApdu, APDU& secureApdu, const SecurityControl &secCtrl);

    void encryptAesCbc(uint8_t* buffer, uint16_t bufLen, const uint8_t* iv, const uint8_t* key);
    void xcryptAesCtr(uint8_t* buffer, uint16_t bufLen, const uint8_t* iv, const uint8_t* key);

    bool _securityModeEnabled {false};

    bool _syncReqBroadcastIncoming{false};
    bool _syncReqBroadcastOutgoing{false};
    uint32_t _lastSyncRes;

    Map<Addr, uint64_t, 1> _pendingOutgoingSyncRequests; // Store challenges for outgoing sync requests
    Map<Addr, uint64_t, 1> _pendingIncomingSyncRequests; // Store challenges for incoming sync requests

    SecurityInterfaceObject& _secIfObj;
    DeviceObject& _deviceObj;
};
