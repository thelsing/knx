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
 * It also takes calls from TransportLayer, decodes the submitted APDU and calls the corresponding
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
    ApplicationLayer(BusAccessUnit& bau);
    /**
     * Assigns the TransportLayer to which encoded APDU are submitted to.
     */
    void transportLayer(TransportLayer& layer);

    void associationTableObject(AssociationTableObject& assocTable);

    // from transport layer
    // Note: without data secure feature, the application layer is just used with SecurityControl.dataSecurity = None
    // hooks that can be implemented by derived class (e.g. SecureApplicationLayer)

#pragma region Transport - Layer - Callbacks
    /**
     * Somebody send us an APDU via multicast communication. See 3.2 of @cite knx:3/3/4. 
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
    virtual void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu);
    /**
     * Report the status of an APDU that we sent via multicast communication back to us. See 3.2 of @cite knx:3/3/4. 
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
    virtual void dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
    virtual void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    virtual void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, bool status);
    virtual void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    virtual void dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, bool status);
    virtual void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu);
    virtual void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, bool status);
    virtual void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu);
    virtual void dataConnectedConfirm(uint16_t tsap);
    void connectIndication(uint16_t tsap);
    void connectConfirm(uint16_t destination, uint16_t tsap, bool status);
    void disconnectIndication(uint16_t tsap);
    void disconnectConfirm(Priority priority, uint16_t tsap, bool status);
#pragma endregion

#pragma region from bau
    void groupValueReadRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl);
    void groupValueReadResponse(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl& secCtrl, uint8_t* data, uint8_t dataLength);
    void groupValueWriteRequest(AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl &secCtrl, uint8_t* data, uint8_t dataLength);
    void individualAddressWriteRequest(AckType ack, HopCountType hopType, const SecurityControl &secCtrl, uint16_t newaddress);
    void individualAddressReadRequest(AckType ack, HopCountType hopType, const SecurityControl& secCtrl);
    void individualAddressReadResponse(AckType ack, HopCountType hopType, const SecurityControl& secCtrl);
    void individualAddressSerialNumberReadRequest(AckType ack, HopCountType hopType, const SecurityControl& secCtrl, uint8_t* serialNumber);
    void individualAddressSerialNumberReadResponse(AckType ack, HopCountType hopType, const SecurityControl& secCtrl, uint8_t* serialNumber,
                                                   uint16_t domainAddress);
    void individualAddressSerialNumberWriteRequest(AckType ack, HopCountType hopType, const SecurityControl& secCtrl, uint8_t* serialNumber,
                                                   uint16_t newaddress);
    void deviceDescriptorReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                     uint8_t descriptorType);
    void deviceDescriptorReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                      uint8_t descriptorType, uint8_t* deviceDescriptor);
    void connectRequest(uint16_t destination, Priority priority);
    void disconnectRequest(Priority priority);
    bool isConnected();
    void restartRequest(AckType ack, Priority priority, HopCountType hopType, const SecurityControl& secCtrl);
    void restartResponse(AckType ack, Priority priority, HopCountType hopType, const SecurityControl& secCtrl, uint8_t errorCode, uint16_t processTime);
    void propertyValueReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                  uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex);
    void propertyValueReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t objectIndex,
                                   uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void propertyValueExtReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                      uint16_t objectType, uint8_t objectInstance, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void propertyValueExtWriteConResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                          uint16_t objectType, uint8_t objectInstance, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t returnCode);
    void propertyValueWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t objectIndex,
                                   uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void adcReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                                     uint8_t channelNr, uint8_t readCount, int16_t value);
    void functionPropertyStateResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                                       uint8_t objectIndex, uint8_t propertyId, uint8_t *resultData, uint8_t resultLength);
    void functionPropertyExtStateResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                          uint16_t objectType, uint8_t objectInstance, uint16_t propertyId, uint8_t* resultData, uint8_t resultLength);
    void propertyDescriptionReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                        uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex);
    void propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                         uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type,
                                         uint16_t maxNumberOfElements, uint8_t access);
    void memoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t number,
                           uint16_t memoryAddress);
    void memoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t number,
                            uint16_t memoryAddress, uint8_t* data);
    void memoryExtReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, ReturnCodes code, uint8_t number,
                            uint32_t memoryAddress, uint8_t* data);
    void memoryExtWriteResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, ReturnCodes code, uint8_t number,
                                uint32_t memoryAddress, uint8_t *memoryData);
    void memoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t number,
                            uint16_t memoryAddress, uint8_t* data);
    void userMemoryReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                               uint32_t memoryAddress);
    void userMemoryReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl, uint8_t number,
                                uint32_t memoryAddress, uint8_t* memoryData);
    void userMemoryWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t number,
                                uint32_t memoryAddress, uint8_t* memoryData);
    void userManufacturerInfoReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl);
    void userManufacturerInfoReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                                          uint8_t* info);
    void authorizeRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint32_t key);
    void authorizeResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t level);
    void keyWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t level, uint32_t key);
    void keyWriteResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t level);

    void systemNetworkParameterReadResponse(Priority priority, HopCountType hopType, const SecurityControl& secCtrl, uint16_t objectType,
                                            uint16_t propertyId, uint8_t* testInfo, uint16_t testInfoLength,
                                            uint8_t* testResult, uint16_t testResultLength);
    void domainAddressSerialNumberReadResponse(Priority priority, HopCountType hopType, const SecurityControl& secCtrl, const uint8_t* rfDoA,
                                               const uint8_t* knxSerialNumber);                                       
    void IndividualAddressSerialNumberReadResponse(Priority priority, HopCountType hopType, const SecurityControl& secCtrl, const uint8_t* domainAddress,
                                               const uint8_t* knxSerialNumber);                                       
#pragma endregion

  protected:
#pragma region hooks
    void dataGroupIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl &secCtrl);
    void dataGroupConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap,
                          APDU& apdu, const SecurityControl& secCtrl, bool status);
    void dataBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu, const SecurityControl& secCtrl);
    void dataBroadcastConfirm(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl, bool status);
    void dataSystemBroadcastIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu, const SecurityControl& secCtrl);
    void dataSystemBroadcastConfirm(HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl, bool status);
    void dataIndividualIndication(HopCountType hopType, Priority priority, uint16_t source, APDU& apdu, const SecurityControl& secCtrl);
    void dataIndividualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl& secCtrl, bool status);
    void dataConnectedIndication(Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl& secCtrl);
    void dataConnectedConfirm(uint16_t tsap, const SecurityControl& secCtrl);
#pragma endregion

    // to transport layer
    virtual void dataGroupRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl &secCtrl);
    virtual void dataBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl);
    virtual void dataSystemBroadcastRequest(AckType ack, HopCountType hopType, Priority priority, APDU& apdu, const SecurityControl& secCtrl);
    virtual void dataIndividualRequest(AckType ack, HopCountType hopType, Priority priority, uint16_t destination, APDU& apdu, const SecurityControl& secCtrl);
    virtual void dataConnectedRequest(uint16_t tsap, Priority priority, APDU& apdu, const SecurityControl& secCtrl); // apdu must be valid until it was confirmed

    uint16_t getConnectedTsasp() {return _connectedTsap;}

    // Protected: we need to access it in derived class SecureApplicationLayer
    TransportLayer* _transportLayer = 0;

    static const SecurityControl noSecurity;

  private:
    void propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl &secCtrl,
                          uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data,
                          uint8_t length);
    void propertyExtDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                             uint16_t objectType, uint8_t objectInstance, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length);
    void memorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl, uint8_t number,
                    uint16_t memoryAddress, uint8_t* memoryData);
    void userMemorySend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, const SecurityControl& secCtrl,
                        uint8_t number, uint32_t memoryAddress, uint8_t* memoryData);
    void groupValueSend(ApduType type, AckType ack, uint16_t asap, Priority priority, HopCountType hopType, const SecurityControl& secCtrl, uint8_t* data, uint8_t& dataLength);
    void individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl &secCtrl);
    void individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU& apdu, const SecurityControl& secCtrl, bool status);
    void individualSend(AckType ack, HopCountType hopType, Priority priority, uint16_t asap, APDU& apdu, const SecurityControl& secCtrl);

    uint16_t _savedAsapReadRequest = 0;
    uint16_t _savedAsapWriteRequest = 0;
    uint16_t _savedAsapResponse = 0;
    AssociationTableObject* _assocTable = nullptr;
    BusAccessUnit& _bau;

    int32_t _connectedTsap = -1;
};
