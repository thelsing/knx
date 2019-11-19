#include "cemi_server.h"
#include "cemi_frame.h"
#include "bau_systemB.h"
#include "usb_tunnel_interface.h"
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
    // The cEMI server will hand out the device address + 1 to the cEMI client (e.g. ETS),
    // so that the device and the cEMI client/server connection(tunnel) can operate simultaneously.
    _clientAddress = _bau.deviceObject().induvidualAddress() + 1;
}

void CemiServer::dataLinkLayer(DataLinkLayer& layer)
{
    _dataLinkLayer = &layer;
}

uint16_t CemiServer::clientAddress() const
{
    return _clientAddress;
}

void CemiServer::clientAddress(uint16_t value)
{
    _clientAddress = value;
}

void CemiServer::dataConfirmationToTunnel(CemiFrame& frame)
{
    print("L_data_con: src: ");
    print(frame.sourceAddress(), HEX);
    print(" dst: ");
    print(frame.destinationAddress(), HEX);

    printHex(" frame: ", frame.data(), frame.dataLength());

    _usbTunnelInterface.sendCemiFrame(frame);
}

void CemiServer::dataIndicationToTunnel(CemiFrame& frame)
{
    static uint8_t lfn = 0;
    uint8_t data[frame.dataLength() + 10];
    data[0] = L_data_ind;
    data[1] = 10;
    data[2] = 0x02; // RF Info add. info
    data[3] = 0x08; // RF info length
    data[4] = 0x02; // RF info field (batt ok)
    pushByteArray(_bau.deviceObject().rfDomainAddress(), 6, &data[5]);
    data[11] = lfn;
    lfn = (lfn + 1) & 0x7; 
    memcpy(&data[12], &frame.data()[2], frame.dataLength() - 2);

    CemiFrame tmpFrame(data, sizeof(data));

    print("L_data_ind: src: ");
    print(tmpFrame.sourceAddress(), HEX);
    print(" dst: ");
    print(tmpFrame.destinationAddress(), HEX);

    printHex(" frame: ", tmpFrame.data(), tmpFrame.dataLength());

    _usbTunnelInterface.sendCemiFrame(tmpFrame);
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
            // Fill in the cEMI client address if the client sets 
            // source address to 0.
            if(frame.sourceAddress() == 0x0000)
            {
                frame.sourceAddress(_clientAddress);
            }

            print("L_data_req: src: ");
            print(frame.sourceAddress(), HEX);
            print(" dst: ");
            print(frame.destinationAddress(), HEX);

            printHex(" frame: ", frame.data(), frame.dataLength());

            _dataLinkLayer->dataRequestFromTunnel(frame);
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

            // Patch result for device address in device object
            // The cEMI server will hand out the device address + 1 to the cEMI client (e.g. ETS),
            // so that the device and the cEMI client/server connection(tunnel) can operate simultaneously.
            // KNX IP Interfaces which offer multiple simultaneous tunnel connections seem to operate the same way.
            // Each tunnel has its own cEMI client address which is based on the main device address.
            if (((ObjectType) objectType == OT_DEVICE) && 
                             (propertyId == PID_DEVICE_ADDR) &&
                             (numberOfElements == 1))
            {
                data[0] = (uint8_t) (_clientAddress & 0xFF);
            }
            else if (((ObjectType) objectType == OT_DEVICE) && 
                             (propertyId == PID_SUBNET_ADDR) &&
                             (numberOfElements == 1))
            {
                data[0] = (uint8_t) ((_clientAddress >> 8) & 0xFF);
            }

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

                delete[] data;
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

            // Patch request for device address in device object
            if (((ObjectType) objectType == OT_DEVICE) && 
                             (propertyId == PID_DEVICE_ADDR) &&
                             (numberOfElements == 1))
            {
                // Temporarily store new cEMI client address in memory
                // We also be sent back if the client requests it again
                _clientAddress = (_clientAddress & 0xFF00) | requestData[0];
                print("cEMI client address: ");
                println(_clientAddress, HEX);
            }
            else if (((ObjectType) objectType == OT_DEVICE) && 
                             (propertyId == PID_SUBNET_ADDR) &&
                             (numberOfElements == 1))
            {
                // Temporarily store new cEMI client address in memory
                // We also be sent back if the client requests it again
                _clientAddress = (_clientAddress & 0x00FF) | (requestData[0] << 8);
                print("cEMI client address: ");
                println(_clientAddress, HEX);
            }            
            else
            {
                _bau.propertyValueWrite((ObjectType)objectType, objectInstance, propertyId, numberOfElements, startIndex, requestData, requestDataSize);
            }

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
            println("M_FuncPropCommand_req not implemented");  
            break;
        }

        case M_FuncPropStateRead_req:
        {
            println("M_FuncPropStateRead_req not implemented");  
            break;
        }

        case M_Reset_req:
        {
            println("M_Reset_req");  
            // A real device reset does not work for USB or KNXNET/IP.
            // Thus, M_Reset_ind is NOT mandatory for USB and KNXNET/IP.
            // We just save all data to the EEPROM
            _bau.writeMemory();
            break;
        }

        // we should never receive these: server -> client
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
