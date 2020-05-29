#include "data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_server.h"

DataLinkLayer::DataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, 
    NetworkLayer& layer, Platform& platform) :
    _deviceObject(devObj), _groupAddressTable(addrTab),  _networkLayer(layer), _platform(platform)
{
}

#ifdef USE_CEMI_SERVER

void DataLinkLayer::cemiServer(CemiServer& cemiServer)
{
    _cemiServer = &cemiServer;
}

void DataLinkLayer::dataRequestFromTunnel(CemiFrame& frame)
{
    _cemiServer->dataConfirmationToTunnel(frame);

    frame.messageCode(L_data_ind);
    
    // Send to local stack
    frameRecieved(frame);

    // Send to KNX medium
    sendFrame(frame);
}
#endif

void DataLinkLayer::dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format, Priority priority, NPDU& npdu)
{
    // Normal data requests and broadcasts will always be transmitted as (domain) broadcast with domain address for open media (e.g. RF medium) 
    // The domain address "simulates" a closed medium (such as TP) on an open medium (such as RF or PL)
    // See 3.2.5 p.22
    sendTelegram(npdu, ack, destinationAddr, addrType, format, priority, Broadcast);
}

void DataLinkLayer::systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu)
{
    // System Broadcast requests will always be transmitted as broadcast with KNX serial number for open media (e.g. RF medium) 
    // See 3.2.5 p.22
#if (MEDIUM_TYPE == 5)||(MEDIUM_TYPE == 0)
    sendTelegram(npdu, ack, 0, GroupAddress, format, priority, Broadcast);
#else
    sendTelegram(npdu, ack, 0, GroupAddress, format, priority, SysBroadcast);
#endif
}

void DataLinkLayer::dataConReceived(CemiFrame& frame, bool success)
{
    MessageCode backupMsgCode = frame.messageCode();
    frame.messageCode(L_data_con);
    frame.confirm(success ? ConfirmNoError : ConfirmError);
    AckType ack = frame.ack();
    AddressType addrType = frame.addressType();
    uint16_t destination = frame.destinationAddress();
    uint16_t source = frame.sourceAddress();
    FrameFormat type = frame.frameType();
    Priority priority = frame.priority();
    NPDU& npdu = frame.npdu();
    SystemBroadcast systemBroadcast = frame.systemBroadcast();

#ifdef USE_CEMI_SERVER
    // if the confirmation was caused by a tunnel request then
    // do not send it to the local stack
    if (frame.sourceAddress() == _cemiServer->clientAddress())
    {
        // Stop processing here and do NOT send it the local network layer
        return;
    }
#endif    

    if (addrType == GroupAddress && destination == 0)
    {
#if (MEDIUM_TYPE == 5)||(MEDIUM_TYPE == 0)
        (void) systemBroadcast; // not used
        _networkLayer.broadcastConfirm(ack, type, priority, source, npdu, success);
#else
        if (systemBroadcast == SysBroadcast)
            _networkLayer.systemBroadcastConfirm(ack, type, priority, source, npdu, success);
        else
            _networkLayer.broadcastConfirm(ack, type, priority, source, npdu, success);
#endif
    }
    else
        _networkLayer.dataConfirm(ack, addrType, destination, type, priority, source, npdu, success);

    frame.messageCode(backupMsgCode);
}

void DataLinkLayer::frameRecieved(CemiFrame& frame)
{
    AckType ack = frame.ack();
    AddressType addrType = frame.addressType();
    uint16_t destination = frame.destinationAddress();
    uint16_t source = frame.sourceAddress();
    FrameFormat type = frame.frameType();
    Priority priority = frame.priority();
    NPDU& npdu = frame.npdu();
    uint16_t ownAddr = _deviceObject.induvidualAddress();
    SystemBroadcast systemBroadcast = frame.systemBroadcast();

#ifdef USE_CEMI_SERVER
    // Do not send our own message back to the tunnel
    if (frame.sourceAddress() != _cemiServer->clientAddress())
    {
        _cemiServer->dataIndicationToTunnel(frame);
    }
#endif

    if (source == ownAddr)
        _deviceObject.induvidualAddressDuplication(true);

    if (addrType == GroupAddress && destination == 0)
    {
#if (MEDIUM_TYPE == 5)||(MEDIUM_TYPE == 0)
        (void) systemBroadcast; // not used
        _networkLayer.broadcastIndication(ack, type, npdu, priority, source);
#else
        if (systemBroadcast == SysBroadcast)
            _networkLayer.systemBroadcastIndication(ack, type, npdu, priority, source);
        else
            _networkLayer.broadcastIndication(ack, type, npdu, priority, source);
#endif
    }
    else
    {
        if (addrType == InduvidualAddress && destination != _deviceObject.induvidualAddress())
            return;

        if (addrType == GroupAddress && !_groupAddressTable.contains(destination))
            return;

//        if (frame.npdu().octetCount() > 0)
//        {
//            _print("-> DLL ");
//            frame.apdu().printPDU();
//        }

        _networkLayer.dataIndication(ack, addrType, destination, type, npdu, priority, source);
    }
}

bool DataLinkLayer::sendTelegram(NPDU & npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority, SystemBroadcast systemBroadcast)
{
    CemiFrame& frame = npdu.frame();
    frame.messageCode(L_data_ind);
    frame.destinationAddress(destinationAddr);
    frame.sourceAddress(_deviceObject.induvidualAddress());
    frame.addressType(addrType);
    frame.priority(priority);
    frame.repetition(RepititionAllowed);
    frame.systemBroadcast(systemBroadcast);

    if (npdu.octetCount() <= 15)
        frame.frameType(StandardFrame);
    else
        frame.frameType(format);


    if (!frame.valid())
    {
        println("invalid frame");
        return false;
    }

//    if (frame.npdu().octetCount() > 0)
//    {
//        _print("<- DLL ");
//        frame.apdu().printPDU();
//    }

    // The data link layer might be an open media link layer
    // and will setup rfSerialOrDoA, rfInfo and rfLfn that we also 
    // have to send through the cEMI server tunnel
    // Thus, reuse the modified cEMI frame as "frame" is only passed by reference here!
    bool success = sendFrame(frame);

#ifdef USE_CEMI_SERVER
    CemiFrame tmpFrame(frame.data(), frame.totalLenght());
    // We can just copy the pointer for rfSerialOrDoA as sendFrame() sets
    // a pointer to const uint8_t data in either device object (serial) or
    // RF medium object (domain address)
    tmpFrame.rfSerialOrDoA(frame.rfSerialOrDoA()); 
    tmpFrame.rfInfo(frame.rfInfo());
    tmpFrame.rfLfn(frame.rfLfn());
    tmpFrame.confirm(ConfirmNoError);
    _cemiServer->dataIndicationToTunnel(tmpFrame);
#endif

    return success;
}

uint8_t* DataLinkLayer::frameData(CemiFrame& frame)
{
    return frame._data;
}


