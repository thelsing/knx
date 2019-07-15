#include "data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"


DataLinkLayer::DataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, 
    NetworkLayer& layer, Platform& platform) :
    _deviceObject(devObj), _groupAddressTable(addrTab),  _networkLayer(layer), _platform(platform)
{
}

void DataLinkLayer::dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format, Priority priority, NPDU& npdu)
{
    sendTelegram(npdu, ack, destinationAddr, addrType, format, priority);
}

void DataLinkLayer::systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu)
{
    sendTelegram(npdu, ack, 0, GroupAddress, format, priority);
}

void DataLinkLayer::dataConReceived(CemiFrame& frame,bool success)
{
    AckType ack = frame.ack();
    AddressType addrType = frame.addressType();
    uint16_t destination = frame.destinationAddress();
    uint16_t source = frame.sourceAddress();
    FrameFormat type = frame.frameType();
    Priority priority = frame.priority();
    NPDU& npdu = frame.npdu();

    if (addrType == GroupAddress && destination == 0)
        _networkLayer.systemBroadcastConfirm(ack, type, priority, source, npdu, success);
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
    
    if (source == ownAddr)
        _deviceObject.induvidualAddressDuplication(true);

    if (addrType == GroupAddress && destination == 0)
        _networkLayer.systemBroadcastIndication(ack, type, npdu, priority, source);
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

bool DataLinkLayer::sendTelegram(NPDU & npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority)
{
    CemiFrame& frame = npdu.frame();
    frame.messageCode(L_data_ind);
    frame.destinationAddress(destinationAddr);
    frame.sourceAddress(_deviceObject.induvidualAddress());
    frame.addressType(addrType);
    frame.priority(priority);
    frame.repetition(RepititionAllowed);

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

    return sendFrame(frame);
}

uint8_t* DataLinkLayer::frameData(CemiFrame& frame)
{
    return frame._data;
}


