#include "data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"

//#define DEBUG_DATA_LINK_LAYER

DataLinkLayer::DataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, 
    NetworkLayer& layer, Platform& platform) :
    _deviceObject(devObj), _groupAddressTable(addrTab),  _networkLayer(layer), _platform(platform)
{
}

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
    sendTelegram(npdu, ack, 0, GroupAddress, format, priority, SysBroadcast);
}

void DataLinkLayer::dataConReceived(CemiFrame& frame, bool success)
{
    AckType ack = frame.ack();
    AddressType addrType = frame.addressType();
    uint16_t destination = frame.destinationAddress();
    uint16_t source = frame.sourceAddress();
    FrameFormat type = frame.frameType();
    Priority priority = frame.priority();
    NPDU& npdu = frame.npdu();
    SystemBroadcast systemBroadcast = frame.systemBroadcast();

    if (addrType == GroupAddress && destination == 0)
            if (systemBroadcast == SysBroadcast)
                _networkLayer.systemBroadcastConfirm(ack, type, priority, source, npdu, success);
            else
                _networkLayer.broadcastConfirm(ack, type, priority, source, npdu, success);                    
    else
        _networkLayer.dataConfirm(ack, addrType, destination, type, priority, source, npdu, success);


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

#ifdef DEBUG_DATA_LINK_LAYER
    println("Link RX:");
    print("\tSRC: ");
    print((source >> 12) & 0x0F);
    print(".");
    print((source >> 8) & 0x0F);
    print(".");
    println(source & 0xFF);

    if (addrType == InduvidualAddress)
    {
        print("\tDST: ");
        print((destination >> 12) & 0x0F);
        print(".");
        print((destination >> 8) & 0x0F);
        print(".");
        println(destination & 0xFF);
    }
    else
    {
        if(destination == 0)
        {
            println("\tSystem Broadcast");
        }
        else
        {
            print("\tDST: ");
            print((destination >> 11) & 0x0F);
            print("/");
            print((destination >> 8) & 0x07);
            print("/");
            println(destination & 0xFF);
        }
    }
#endif

    if (source == ownAddr)
        _deviceObject.induvidualAddressDuplication(true);

    if (addrType == GroupAddress && destination == 0)
    {
        if (systemBroadcast == SysBroadcast)
            _networkLayer.systemBroadcastIndication(ack, type, npdu, priority, source);
        else 
            _networkLayer.broadcastIndication(ack, type, npdu, priority, source);
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

#ifdef DEBUG_DATA_LINK_LAYER
    uint16_t source = _deviceObject.induvidualAddress();

    println("Link TX:");
    print("\tSRC: ");
    print((source >> 12) & 0x0F);
    print(".");
    print((source >> 8) & 0x0F);
    print(".");
    println(source & 0xFF);

    if (addrType == InduvidualAddress)
    {
        print("\tDST: ");
        print((destinationAddr >> 12) & 0x0F);
        print(".");
        print((destinationAddr >> 8) & 0x0F);
        print(".");
        println(destinationAddr & 0xFF);
    }
    else
    {
        if(destinationAddr == 0)
        {
            println("\tSystem Broadcast");
        }
        else
        {
            print("\tDST: ");
            print((destinationAddr >> 11) & 0x0F);
            print("/");
            print((destinationAddr >> 8) & 0x07);
            print("/");
            println(destinationAddr & 0xFF);
        }
    }
#endif
//    if (frame.npdu().octetCount() > 0)
//    {
//        _print("<- DLL ");
//        frame.apdu().printPDU();
//    }

    return sendFrame(frame);
}

uint8_t* DataLinkLayer::frameData(CemiFrame& frame)
{
    return frame._data;
}


