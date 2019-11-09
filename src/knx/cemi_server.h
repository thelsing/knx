#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "usb_data_link_layer.h"

class BusAccessUnit;
class DataLinkLayer;
class CemiFrame;

/**
 * This is an implementation of the cEMI server as specified in @cite knx:3/6/3.
 * Overview on page 57.
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
     * @param tunnelInterface The TunnelInterface of the KNX tunnel (e.g. USB or KNXNET/IP)
     * @param bau methods are called here depending of the content of the APDU
     */
    CemiServer(BusAccessUnit& bau);

    void dataLinkLayer(DataLinkLayer& layer);

    // from data link layer
    // Only L_Data service
    void dataIndicationToTunnel(CemiFrame& frame);

    // From tunnel interface
    void frameReceived(CemiFrame& frame);


/*
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
*/
  private:
/*  
    void propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                          uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data,
                          uint8_t length);
    void individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
*/
    DataLinkLayer* _dataLinkLayer;
    BusAccessUnit& _bau;
    UsbDataLinkLayer _usbTunnelInterface;
};
