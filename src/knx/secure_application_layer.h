#pragma once

#include "application_layer.h"
#include <stdint.h>
#include "knx_types.h"
#include "apdu.h"

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
    SecureApplicationLayer(AssociationTableObject& assocTable, BusAccessUnit& bau);

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
    virtual void connectIndication(uint16_t tsap) override;
    virtual void connectConfirm(uint16_t destination, uint16_t tsap, bool status) override;
    virtual void disconnectIndication(uint16_t tsap) override;
    virtual void disconnectConfirm(Priority priority, uint16_t tsap, bool status) override;
    virtual void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu) override;
    virtual void dataConnectedConfirm(uint16_t tsap) override;

  protected:
    // to transport layer
    virtual void dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu) override;
    virtual void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu) override;
    virtual void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu) override;
    virtual void dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu) override;
    virtual void dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu) override; // apdu must be valid until it was confirmed

  private:
    uint32_t calcAuthOnlyMac(uint8_t* apdu, uint8_t apduLength, uint8_t* key, uint8_t* iv, uint8_t* ctr0);
    uint32_t calcConfAuthMac(uint8_t* associatedData, uint16_t associatedDataLength, uint8_t* apdu, uint8_t apduLength, uint8_t* key, uint8_t* iv);

    void block0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr, uint8_t extFrameFormat, uint8_t tpci, uint8_t apci, uint8_t payloadLength);
    void blockCtr0(uint8_t* buffer, uint8_t* seqNum, uint16_t indSrcAddr, uint16_t dstAddr, bool dstAddrIsGroupAddr);

    uint64_t lastValidSequenceNumber(bool toolAcces, uint16_t srcAddr);

    bool decrypt(uint8_t* plainApdu, uint16_t srcAddr, uint16_t dstAddr, uint8_t tpci, uint8_t* secureAsdu, uint16_t secureAdsuLength);
    void encrypt(uint8_t* buffer, uint16_t srcAddr, uint16_t dstAddr, uint8_t tpci, uint8_t* apdu, uint16_t apduLength);

    // Our FDSK
    static uint8_t _key[];

};