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
    void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap,
                          APDU& apdu, bool status);
    void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status);
    void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status);
    void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
    void connectIndication(uint16_t tsap);
    void connectConfirm(uint16_t destination, uint16_t tsap, bool status);
    void disconnectIndication(uint16_t tsap);
    void disconnectConfirm(Priority priority, uint16_t tsap, bool status);
    void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu);
    void dataConnectedConfirm(uint16_t tsap);

  protected:
    // to transport layer
    virtual void dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    virtual void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu);
    virtual void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu);
    virtual void dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu);
    virtual void dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu); // apdu must be valid until it was confirmed

  private:
};
