#include "cemi_server.h"
#include "cemi_frame.h"
#include "bau_systemB.h"
#include "usb_data_link_layer.h"
#include "data_link_layer.h"
#include "string.h"
#include "bits.h"
#include <stdio.h>

CemiServer::CemiServer(BauSystemB& bau)
    : _bau(bau),
      _usbTunnelInterface(*this,
        _bau.deviceObject().maskVersion(),
        _bau.deviceObject().manufacturerId())
{
}

void CemiServer::dataLinkLayer(DataLinkLayer& layer)
{
    _dataLinkLayer = &layer;
}

void CemiServer::dataIndicationToTunnel(CemiFrame& frame)
{
    println("L_data_ind: ");
    _usbTunnelInterface.sendCemiFrame(frame);
}

/*
void CemiServer::localManagmentRequestFromTunnel(CemiFrame& frame)
{
    // called from KNXNET/IP for Management Services

    // Send response to IP data link layer 
    _dataLinkLayer->localManagmentResponseToTunnel();
}
*/

void CemiServer::frameReceived(CemiFrame& frame)
{
    switch(frame.messageCode())
    {
        case L_data_req:
        {
            println("L_data_req: ");

            // Send as indication to data link layer
            frame.messageCode(L_data_ind);
            _dataLinkLayer->dataIndicationFromTunnel(frame);
            
            // Send local reply to tunnel
            frame.messageCode(L_data_con);
            _usbTunnelInterface.sendCemiFrame(frame);
            break;
        }

        case M_PropRead_req:
        {
            print("M_PropRead_req: ");
            
            uint16_t objectType;
            popWord(objectType, &frame.data()[1]);
            uint8_t objectInstance = frame.data()[3];
            uint8_t propertyId = frame.data()[4];
            uint32_t numberOfElements = frame.data()[5] >> 4;
            uint16_t startIndex = frame.data()[6] | ((frame.data()[5]&0x0F)<<8);
            uint8_t* data = nullptr;
            uint32_t dataSize = 0;

            print("ObjType: ");
            print(objectType, DEC);
            print(" ObjInst: ");
            print(objectInstance, DEC);
            print(" PropId: ");
            print(propertyId, DEC);
            print(" NoE: ");
            print(numberOfElements, DEC);
            print(" startIdx: ");
            print(startIndex, DEC);

            // propertyValueRead() allocates memory for the data! Needs to be deleted again!
            _bau.propertyValueRead((ObjectType)objectType, objectInstance, propertyId, numberOfElements, startIndex, &data, dataSize);

            if (data && dataSize && numberOfElements)
            {
                printHex(" <- data: ", data, dataSize);
                println("");

                // Prepare positive response
                uint8_t responseData[7 + dataSize];
                memcpy(responseData, frame.data(), 7);
                memcpy(&responseData[7], data, dataSize); 
                
                CemiFrame responseFrame(responseData, sizeof(responseData));
                responseFrame.messageCode(M_PropRead_con);
                _usbTunnelInterface.sendCemiFrame(responseFrame);

                delete data;
            }
            else
            {
                // Prepare negative response
                uint8_t responseData[7 + 1];
                memcpy(responseData, frame.data(), sizeof(responseData));
                responseData[7] = Void_DP; // Set cEMI error code
                responseData[5] = 0; // Set Number of elements to zero

                printHex(" <- error: ", &responseData[7], 1);
                println("");

                CemiFrame responseFrame(responseData, sizeof(responseData));
                responseFrame.messageCode(M_PropRead_con);
                _usbTunnelInterface.sendCemiFrame(responseFrame);
            }
            break;
        }

        case M_PropWrite_req:
        {
            print("M_PropWrite_req: "); 

            uint16_t objectType;
            popWord(objectType, &frame.data()[1]);
            uint8_t objectInstance = frame.data()[3];
            uint8_t propertyId = frame.data()[4];
            uint32_t numberOfElements = frame.data()[5] >> 4;
            uint16_t startIndex = frame.data()[6] | ((frame.data()[5]&0x0F)<<8);
            uint8_t* requestData = &frame.data()[7];
            uint32_t requestDataSize = frame.dataLength() - 7;

            print("ObjType: ");
            print(objectType, DEC);
            print(" ObjInst: ");
            print(objectInstance, DEC);
            print(" PropId: ");
            print(propertyId, DEC);
            print(" NoE: ");
            print(numberOfElements, DEC);
            print(" startIdx: ");
            print(startIndex, DEC);

            printHex(" -> data: ", requestData, requestDataSize);

            _bau.propertyValueWrite((ObjectType)objectType, objectInstance, propertyId, numberOfElements, startIndex, requestData, requestDataSize);

            if (numberOfElements)
            {
                // Prepare positive response
                uint8_t responseData[7];
                memcpy(responseData, frame.data(), sizeof(responseData));

                println(" <- no error");

                CemiFrame responseFrame(responseData, sizeof(responseData));
                responseFrame.messageCode(M_PropWrite_con);
                _usbTunnelInterface.sendCemiFrame(responseFrame);
            }
            else
            {
                // Prepare negative response
                uint8_t responseData[7 + 1];
                memcpy(responseData, frame.data(), sizeof(responseData));
                responseData[7] = Illegal_Command; // Set cEMI error code
                responseData[5] = 0; // Set Number of elements to zero

                printHex(" <- error: ", &responseData[7], 1);
                println("");

                CemiFrame responseFrame(responseData, sizeof(responseData));
                responseFrame.messageCode(M_PropWrite_con);
                _usbTunnelInterface.sendCemiFrame(responseFrame);
            }
            break;
        }

        case M_FuncPropCommand_req:
        {
            println("M_FuncPropCommand_req");  
            break;
        }

        case M_FuncPropStateRead_req:
        {
            println("M_FuncPropStateRead_req");  
            break;
        }

        case M_Reset_req:
        {
            println("M_Reset_req");  
            // A real device reset does not work with USB
            // M_Reset_ind is not mandatory for USB and KNXNET/IP
            // Flush the EEPROM before resetting
            //_bau.writeMemory();
            break;
        }

        // we should not receive this: server -> client
        case L_data_con:
        case L_data_ind:
        case M_PropInfo_ind:
        case M_PropRead_con:
        case M_PropWrite_con:
        case M_FuncPropCommand_con:
        //case M_FuncPropStateRead_con: // same value as M_FuncPropCommand_con
        case M_Reset_ind:
        default:
            break;
    }
}


void CemiServer::loop()
{
    _usbTunnelInterface.loop();
}

/*
void CemiServer::propertyValueReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex)
{
    CemiFrame frame(5);
    APDU& apdu = frame.apdu();
    apdu.type(PropertyValueRead);
    uint8_t* data = apdu.data();
    data += 1;
    data = pushByte(objectIndex, data);
    data = pushByte(propertyId, data);
    pushWord(startIndex & 0xfff, data);
    *data &= ((numberOfElements & 0xf) << 4);
    
    individualSend(ack, hopType, priority, asap, apdu);
}

void CemiServer::propertyValueReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    propertyDataSend(PropertyValueResponse, ack, priority, hopType, asap, objectIndex, propertyId, numberOfElements,
        startIndex, data, length);
}

void CemiServer::propertyValueWriteRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t * data, uint8_t length)
{
    propertyDataSend(PropertyValueWrite, ack, priority, hopType, asap, objectIndex, propertyId, numberOfElements,
        startIndex, data, length);
}

void CemiServer::propertyDescriptionReadRequest(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex)
{
    CemiFrame frame(4);
    APDU& apdu = frame.apdu();
    apdu.type(PropertyDescriptionRead);
    uint8_t* data = apdu.data();
    data[1] = objectIndex;
    data[2] = propertyId;
    data[3] = propertyIndex;
    individualSend(ack, hopType, priority, asap, apdu);
}

void CemiServer::propertyDescriptionReadResponse(AckType ack, Priority priority, HopCountType hopType, uint16_t asap,
    uint8_t objectIndex, uint8_t propertyId, uint8_t propertyIndex, bool writeEnable, uint8_t type, 
    uint16_t maxNumberOfElements, uint8_t access)
{
    CemiFrame frame(8);
    APDU& apdu = frame.apdu();
    apdu.type(PropertyDescriptionResponse);
    uint8_t* data = apdu.data();
    data[1] = objectIndex;
    data[2] = propertyId;
    data[3] = propertyIndex;
    if (writeEnable)
        data[4] |= 0x80;
    data[4] |= (type & 0x3f);
    pushWord(maxNumberOfElements & 0xfff, data + 5);
    data[7] = access;
    individualSend(ack, hopType, priority, asap, apdu);
}


void CemiServer::propertyDataSend(ApduType type, AckType ack, Priority priority, HopCountType hopType, uint16_t asap, 
    uint8_t objectIndex, uint8_t propertyId, uint8_t numberOfElements, uint16_t startIndex, uint8_t* data, uint8_t length)
{
    CemiFrame frame(5 + length);
    APDU& apdu = frame.apdu();
    apdu.type(type);
    uint8_t* apduData = apdu.data();
    apduData += 1;
    apduData = pushByte(objectIndex, apduData);
    apduData = pushByte(propertyId, apduData);
    pushWord(startIndex & 0xfff, apduData);
    *apduData |= ((numberOfElements & 0xf) << 4);
    apduData += 2;
    if (length > 0)
        memcpy(apduData, data, length);
}

void CemiServer::individualIndication(HopCountType hopType, Priority priority, uint16_t tsap, APDU & apdu)
{
    uint8_t* data = apdu.data();
    switch (apdu.type())
    {
        case PropertyValueRead:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadIndication(priority, hopType, tsap, data[1], data[2], data[3] >> 4, startIndex);
            break;
        }
        case PropertyValueResponse:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadAppLayerConfirm(priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5);
            break;
        }
        case PropertyValueWrite:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueWriteIndication(priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5);
            break;
        }
        case PropertyDescriptionRead:
            _bau.propertyDescriptionReadIndication(priority, hopType, tsap, data[1], data[2], data[3]);
            break;
        case PropertyDescriptionResponse:
            _bau.propertyDescriptionReadAppLayerConfirm(priority, hopType, tsap, data[1], data[2], data[3],
                (data[4] & 0x80) > 0, data[4] & 0x3f, getWord(data + 5) & 0xfff, data[7]);
            break;
    }
}

void CemiServer::individualConfirm(AckType ack, HopCountType hopType, Priority priority, uint16_t tsap, APDU & apdu, bool status)
{
    uint8_t* data = apdu.data();
    switch (apdu.type())
    {
        case PropertyValueRead:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadLocalConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, status);
            break;
        }
        case PropertyValueResponse:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueReadResponseConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5, status);
            break;
        }
        case PropertyValueWrite:
        {
            uint16_t startIndex;
            popWord(startIndex, data + 3);
            startIndex &= 0xfff;
            _bau.propertyValueWriteLocalConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3] >> 4,
                startIndex, data + 5, apdu.length() - 5, status);
            break;
        }
        case PropertyDescriptionRead:
            _bau.propertyDescriptionReadLocalConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3], status);
            break;
        case PropertyDescriptionResponse:
            _bau.propertyDescriptionReadResponseConfirm(ack, priority, hopType, tsap, data[1], data[2], data[3],
                (data[4] & 0x80) > 0, data[4] & 0x3f, getWord(data + 5) & 0xfff, data[7], status);
            break;
    }
}

*/