#include "config.h"
#ifdef USE_CEMI_SERVER

#include "cemi_server.h"
#include "cemi_frame.h"
#include "bau_systemB.h"
#include "usb_tunnel_interface.h"
#include "data_link_layer.h"
#include "string.h"
#include "bits.h"
#include <stdio.h>

CemiServer::CemiServer(BauSystemB& bau)
    : _bau(bau)
#ifdef USE_USB
        ,
        _usbTunnelInterface(*this,
        _bau.deviceObject().maskVersion(),
        _bau.deviceObject().manufacturerId())
#endif
{
    // The cEMI server will hand out the device address + 1 to the cEMI client (e.g. ETS),
    // so that the device and the cEMI client/server connection(tunnel) can operate simultaneously.
    _clientAddress = _bau.deviceObject().individualAddress() + 1;
}

void CemiServer::dataLinkLayer(DataLinkLayer& layer)
{
    _dataLinkLayer = &layer;
}

#ifdef KNX_TUNNELING
void CemiServer::dataLinkLayerPrimary(DataLinkLayer& layer)
{
    _dataLinkLayerPrimary = &layer;
}

#endif
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
    MessageCode backupMsgCode = frame.messageCode();

    frame.messageCode(L_data_con);

#ifdef KNX_LOG_TUNNELING
    print("L_data_con: src: ");
    print(frame.sourceAddress(), HEX);
    print(" dst: ");
    print(frame.destinationAddress(), HEX);

    printHex(" frame: ", frame.data(), frame.dataLength());
#endif

#ifdef USE_USB
    _usbTunnelInterface.sendCemiFrame(frame);
#elif defined(KNX_TUNNELING)
    _dataLinkLayerPrimary->dataConfirmationToTunnel(frame);
#endif

    frame.messageCode(backupMsgCode);
}

void CemiServer::dataIndicationToTunnel(CemiFrame& frame)
{
#ifdef USE_RF
    bool isRf = _dataLinkLayer->mediumType() == DptMedium::KNX_RF;
    uint8_t data[frame.dataLength() + (isRf ? 10 : 0)];
#else
    uint8_t data[frame.dataLength()];
#endif

#ifdef USE_RF
    if (isRf)
    {
        data[0] = L_data_ind;     // Message Code
        data[1] = 0x0A;           // Total additional info length
        data[2] = 0x02;           // RF add. info: type
        data[3] = 0x08;           // RF add. info: length
        data[4] = frame.rfInfo(); // RF add. info: info field (batt ok, bidir)
        pushByteArray(frame.rfSerialOrDoA(), 6, &data[5]); // RF add. info:Serial or Domain Address
        data[11] = frame.rfLfn(); // RF add. info: link layer frame number
        memcpy(&data[12], &((frame.data())[2]), frame.dataLength() - 2);
    }
    else
    {
#endif
        memcpy(&data[0], frame.data(), frame.dataLength());
#ifdef USE_RF
    }
#endif

    CemiFrame tmpFrame(data, sizeof(data));

#ifdef KNX_LOG_TUNNELING
    print("ToTunnel ");
    print("L_data_ind: src: ");
    print(tmpFrame.sourceAddress(), HEX);
    print(" dst: ");
    print(tmpFrame.destinationAddress(), HEX);

    printHex(" frame: ", tmpFrame.data(), tmpFrame.dataLength());
#endif
    tmpFrame.apdu().type();

#ifdef USE_USB
    _usbTunnelInterface.sendCemiFrame(tmpFrame);
#elif defined(KNX_TUNNELING)
    _dataLinkLayerPrimary->dataIndicationToTunnel(frame);
#endif
}

void CemiServer::frameReceived(CemiFrame& frame)
{
    switch(frame.messageCode())
    {
        case L_data_req:
        {
            handleLData(frame);
            break;
        }

        case M_PropRead_req:
        {
            handleMPropRead(frame);
            break;
        }

        case M_PropWrite_req:
        {
            handleMPropWrite(frame);
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
            handleMReset(frame);
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

void CemiServer::handleLData(CemiFrame& frame)
{
    // Fill in the cEMI client address if the client sets 
    // source address to 0.
#ifndef KNX_TUNNELING
    //We already set the correct IA
    if(frame.sourceAddress() == 0x0000)
    {
        frame.sourceAddress(_clientAddress);
    }
#endif

#ifdef USE_RF
    if (_dataLinkLayer->mediumType() == DptMedium::KNX_RF)
    {
        // Check if we have additional info for RF
        if (((frame.data())[1] == 0x0A) && // Additional info total length: we only handle one additional info of type RF
            ((frame.data())[2] == 0x02) && // Additional info type: RF
            ((frame.data())[3] == 0x08) )  // Additional info length of type RF: 8 bytes (fixed)
        {
            frame.rfInfo((frame.data())[4]);
            // Use the values provided in the RF additonal info
            if ( ((frame.data())[5] != 0x00) || ((frame.data())[6] != 0x00) || ((frame.data())[7] != 0x00) ||
                    ((frame.data())[8] != 0x00) || ((frame.data())[9] != 0x00) || ((frame.data())[10] != 0x00) )
            {
                frame.rfSerialOrDoA(&((frame.data())[5]));
            } // else leave the nullptr as it is
            frame.rfLfn((frame.data())[11]);
        }

        // If the cEMI client does not provide a link layer frame number (LFN),
        // we use our own counter.
        // Note: There is another link layer frame number counter inside the RF data link layer class!
        //       That counter is solely for the local application!
        //       If we set a LFN here, the data link layer counter is NOT used!
        if (frame.rfLfn() == 0xFF)
        {
            // Set Data Link Layer Frame Number
            frame.rfLfn(_frameNumber);
            // Link Layer frame number counts 0..7
            _frameNumber = (_frameNumber + 1) & 0x7;
        }
    }
#endif

#ifdef KNX_LOG_TUNNELING
    print("L_data_req: src: ");
    print(frame.sourceAddress(), HEX);
    print(" dst: ");
    print(frame.destinationAddress(), HEX);
    printHex(" frame: ", frame.data(), frame.dataLength());
#endif
    _dataLinkLayer->dataRequestFromTunnel(frame);
}

void CemiServer::handleMPropRead(CemiFrame& frame)
{
#ifdef KNX_LOG_TUNNELING
    print("M_PropRead_req: ");
#endif
    
    uint16_t objectType;
    popWord(objectType, &frame.data()[1]);
    uint8_t objectInstance = frame.data()[3];
    uint8_t propertyId = frame.data()[4];
    uint8_t numberOfElements = frame.data()[5] >> 4;
    uint16_t startIndex = frame.data()[6] | ((frame.data()[5]&0x0F)<<8);
    uint8_t* data = nullptr;
    uint32_t dataSize = 0;

#ifdef KNX_LOG_TUNNELING
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
#endif

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
#ifdef KNX_LOG_TUNNELING
        printHex(" <- data: ", data, dataSize);
#endif

        // Prepare positive response
        uint8_t responseData[7 + dataSize];
        memcpy(responseData, frame.data(), 7);
        memcpy(&responseData[7], data, dataSize); 

        CemiFrame responseFrame(responseData, sizeof(responseData));
        responseFrame.messageCode(M_PropRead_con);
#ifdef USE_USB
        _usbTunnelInterface.sendCemiFrame(responseFrame);
#elif defined(KNX_TUNNELING)
        _dataLinkLayerPrimary->dataRequestToTunnel(responseFrame);
#endif
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
#ifdef USE_USB
        _usbTunnelInterface.sendCemiFrame(responseFrame);
#elif defined(KNX_TUNNELING)
    _dataLinkLayerPrimary->dataRequestToTunnel(responseFrame);
#endif
    }
}

void CemiServer::handleMPropWrite(CemiFrame& frame)
{
    print("M_PropWrite_req: "); 

    uint16_t objectType;
    popWord(objectType, &frame.data()[1]);
    uint8_t objectInstance = frame.data()[3];
    uint8_t propertyId = frame.data()[4];
    uint8_t numberOfElements = frame.data()[5] >> 4;
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
#ifdef USE_USB
        _usbTunnelInterface.sendCemiFrame(responseFrame);
#elif defined(KNX_TUNNELING)
    _dataLinkLayerPrimary->dataRequestToTunnel(responseFrame);
#endif
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
#ifdef USE_USB
        _usbTunnelInterface.sendCemiFrame(responseFrame);
#elif defined(KNX_TUNNELING)
    _dataLinkLayerPrimary->dataRequestToTunnel(responseFrame);
#endif
    }
}

void CemiServer::handleMReset(CemiFrame& frame)
{
    println("M_Reset_req: sending M_Reset_ind");  
    // A real device reset does not work for USB or KNXNET/IP.
    // Thus, M_Reset_ind is NOT mandatory for USB and KNXNET/IP.
    // We just save all data to the EEPROM
    _bau.writeMemory();
    // Prepare response
    uint8_t responseData[1];
    CemiFrame responseFrame(responseData, sizeof(responseData));
    responseFrame.messageCode(M_Reset_ind);
#ifdef USE_USB
    _usbTunnelInterface.sendCemiFrame(responseFrame);
#elif defined(KNX_TUNNELING)
    _dataLinkLayerPrimary->dataRequestToTunnel(responseFrame);
#endif
}

void CemiServer::loop()
{
#ifdef USE_USB
    _usbTunnelInterface.loop();
#endif
}

#endif
