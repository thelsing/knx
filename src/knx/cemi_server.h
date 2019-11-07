#pragma once

#include <stdint.h>
#include "knx_types.h"

class UsbDataLinkLayer;
class BusAccessUnit;
/**
 * This is an implementation of the cEMI server as specified in @cite knx:3/6/3.
 * It provides methods for the BusAccessUnit to do different things and translates this 
 * call to an cEMI frame and calls the correct method of the data link layer. 
 * It also takes calls from data link layer, decodes the submitted cEMI frames and calls the corresponding
 * methods of the BusAccessUnit class.
 */
class CemiServer
{
  public:
    /**
     * The constructor.
     * @param assocTable The AssociationTable is used to translate between asap (i.e. group objects) and group addresses.
     * @param bau methods are called here depending of the content of the APDU
     */
    CemiServer(UsbDataLinkLayer& dlLayer, BusAccessUnit& bau);

    // from data link layer
#pragma region Data - Link - Layer - Callbacks
    void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
#pragma endregion

#pragma region from bau
    void propertyValueReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                  uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    void propertyValueReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
                                   uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void propertyValueWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t objectIndex,
                                   uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void propertyDescriptionReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex);
    void propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                         uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
                                         uint16_t maxNumberOfElements, uint8_t access);
#pragma endregion

  private:
    void propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                          uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data,
                          uint8_t length);
    void individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);

    UsbDataLinkLayer& _dlLayer;
    BusAccessUnit& _bau;
};
