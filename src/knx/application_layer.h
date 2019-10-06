#pragma once

#include <stdint.h>
#include "knx_types.h"
#include "apdu.h"

class AssociationTableObject;
class BusAccessUnit;
class TransportLayer;
/**
 * This is an implementation of the application layer as specified in @cite knx:3/5/1.
 * It provides methods for the BusAccessUnit to do different things and translates this 
 * call to an APDU and calls the correct method of the TransportLayer. 
 * It also takes calls from TransportLayer, decodes the submitted APDU and calls the coresponding
 * methods of the BusAccessUnit class.
 */
class ApplicationLayer
{
  public:
    /**
     * The constructor.
     * @param assocTable The AssociationTable is used to translate between asap (i.e. group objects) and group addresses.
     * @param bau methods are called here depending of the content of the APDU
     */
    ApplicationLayer(AssociationTableObject& assocTable, BusAccessUnit& bau);
    /**
     * Assigns the TransportLayer to which encoded APDU are submitted to.
     */
    void transportLayer(TransportLayer& layer);

    // from transport layer
#pragma region Transport - Layer - Callbacks
    /**
     * Somebody send us an APDU via multicast communiation. See 3.2 of @cite knx:3/3/4. 
     * See also ApplicationLayer::dataGroupConfirm and TransportLayer::dataGroupRequest.
     * This method is called by the TransportLayer.
     * 
     * @param tsap used the find the correct GroupObject with the help of the AssociationTableObject. 
     *        See 3.1.1 of @cite knx:3/3/7
     *        
     * @param apdu The submitted APDU.
     * 
     * @param priority The ::Priority of the received request.
     * 
     * @param hopType Should routing be endless or should the NetworkLayer::hopCount be used? See also ::HopCountType.
     */
    void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    /**
     * Report the status of an APDU that we sent via multicast communiation back to us. See 3.2 of @cite knx:3/3/4. 
     * See also ApplicationLayer::dataGroupConfirm and TransportLayer::dataGroupRequest. This method is called by 
     * the TransportLayer.
     * 
     * @param tsap used the find the correct GroupObject with the help of the AssociationTableObject. 
     *        See 3.1.1 of @cite knx:3/3/7
     *        
     * @param apdu The submitted APDU.
     * 
     * @param priority The ::Priority of the received request.
     * 
     * @param hopType Should routing be endless or should the NetworkLayer::hopCount be used? See also ::HopCountType.
     * 
     * @param status Was the request successful?
     * 
     * @param ack Did we want a DataLinkLayer acknowledgement? See ::AckType.
     */
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
#pragma endregion

#pragma region from bau
    void groupValueReadRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType);
    void groupValueReadResponse(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, uint8_t dataLength);
    void groupValueWriteRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, uint8_t dataLength);
    void individualAddressWriteRequest(AckType ack, HopCountType hopType, uint16_t newaddress);
    void individualAddressReadRequest(AckType ack, HopCountType hopType);
    void individualAddressReadResponse(AckType ack, HopCountType hopType);
    void individualAddressSerialNumberReadRequest(AckType ack, HopCountType hopType, uint8_t* serialNumber);
    void individualAddressSerialNumberReadResponse(AckType ack, HopCountType hopType, uint8_t* serialNumber,
                                                   uint16_t domainAddress);
    void individualAddressSerialNumberWriteRequest(AckType ack, HopCountType hopType, uint8_t* serialNumber,
                                                   uint16_t newaddress);
    void deviceDescriptorReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                     uint8_t descriptorType);
    void deviceDescriptorReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                      uint8_t descriptorType, uint8_t* deviceDescriptor);
    void connectRequest(uint16_t destination, Priority priority);
    void disconnectRequest(Priority priority);
    bool isConnected();
    void restartRequest(AckType ack, Priority priority, HopCountType hopType);
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
    void memoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                           uint16_t memoryAddress);
    void memoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                            uint16_t memoryAddress, uint8_t* data);
    void memoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                            uint16_t memoryAddress, uint8_t* data);
    void userMemoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                               uint32_t memoryAddress);
    void userMemoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                                uint32_t memoryAddress, uint8_t* memoryData);
    void userMemoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                                uint32_t memoryAddress, uint8_t* memoryData);
    void userManufacturerInfoReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap);
    void userManufacturerInfoReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                                          uint8_t* info);
    void authorizeRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint32_t key);
    void authorizeResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level);
    void keyWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level, uint32_t key);
    void keyWriteResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t level);
#pragma endregion

  private:
    void propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                          uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data,
                          uint8_t length);
    void memorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, uint8_t number,
                    uint16_t memoryAddress, uint8_t* memoryData);
    void userMemorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
                        uint8_t number, uint32_t memoryAddress, uint8_t* memoryData);
    void groupValueSend(ApduType type, AckType ack, uint16_t asap, Priority priority, HopCountType hopType, uint8_t* data, uint8_t& dataLength);
    void individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    void individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
    void individualSend(AckType ack, HopCountType hopType, Priority priority, uint16_t asap, APDU& apdu);

    uint16_t _savedAsapReadRequest;
    uint16_t _savedAsapWriteRequest;
    uint16_t _savedAsapResponse;
    AssociationTableObject& _assocTable;
    BusAccessUnit& _bau;
    TransportLayer* _transportLayer = 0;
    int32_t _connectedTsap = -1;
};
