/*
 *  bus.cpp - Low level EIB bus access.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

#define ROUTING_INDICATION 0x0530

#define KNXIP_MULTICAST_PORT 3671
#define MIN_LEN_CEMI 10

#ifdef DUMP_TELEGRAMS
unsigned char telBuffer[32];
uint32_t telLength = 0;
#endif

DataLinkLayer::DataLinkLayer(DeviceObject& devObj, AddressTableObject& addrTab, IpParameterObject& ipParam, 
    NetworkLayer& layer, Platform& platform) :
    _deviceObject(devObj), _groupAddressTable(addrTab), _ipParameters(ipParam), _networkLayer(layer), _platform(platform)
{
}

void DataLinkLayer::dataRequest(AckType ack, AddressType addrType, uint16_t destinationAddr, FrameFormat format, Priority priority, NPDU& npdu)
{
    bool success = sendPacket(npdu, ack, destinationAddr, addrType, format, priority);
    _networkLayer.dataConfirm(ack, addrType, destinationAddr, format, priority, npdu.frame().sourceAddress(), npdu, success);
}

void DataLinkLayer::systemBroadcastRequest(AckType ack, FrameFormat format, Priority priority, NPDU& npdu)
{
    bool success = sendPacket(npdu, ack, 0, GroupAddress, format, priority);
    _networkLayer.systemBroadcastConfirm(ack, format, priority, npdu.frame().sourceAddress(), npdu, success);
}

bool DataLinkLayer::sendPacket(NPDU &npdu, AckType ack, uint16_t destinationAddr, AddressType addrType, FrameFormat format, Priority priority)
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
        printf("invalid frame\n");
        return false;
    }


    //if (frame.npdu().octetCount() > 0)
    //{
    //    print.print("-> DLL ");
    //    frame.apdu().printPDU();
    //}


    uint16_t length = frame.totalLenght() + KNXIP_HEADER_LEN;
    uint8_t* buffer = new uint8_t[length];
    buffer[0] = KNXIP_HEADER_LEN;
    buffer[1] = KNXIP_PROTOCOL_VERSION;
    pushWord(ROUTING_INDICATION, buffer + 2);
    pushWord(length, buffer + 4);

    memcpy(buffer + KNXIP_HEADER_LEN, frame._data, frame.totalLenght());
    
    bool success = sendBytes(buffer, length);
    // only send 50 packet per second: see KNX 3.2.6 p.6
    _platform.mdelay(20);
    delete[] buffer;
    return success;
}

void DataLinkLayer::loop()
{
    if (!_enabled)
        return;

    uint8_t buffer[512];
    int len = _platform.readBytes(buffer, 512);
    if (len <= 0)
        return;

    if (len < KNXIP_HEADER_LEN)
        return;
    
    if (buffer[0] != KNXIP_HEADER_LEN 
        || buffer[1] != KNXIP_PROTOCOL_VERSION)
        return;

    uint16_t code;
    popWord(code, buffer + 2);
    if (code != ROUTING_INDICATION) // only routing indication for now
        return;
    
    if (len < MIN_LEN_CEMI)
        return;

    //TODO: Check correct length (additions Info + apdu length)
    CemiFrame frame(buffer + KNXIP_HEADER_LEN, len - KNXIP_HEADER_LEN);
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

        //if (frame.npdu().octetCount() > 0)
        //{
        //    print.print("<- DLL ");
        //    frame.apdu().printPDU();
        //}

        _networkLayer.dataIndication(ack, addrType, destination, type, npdu, priority, source);
    }
}

void DataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        _platform.setupMultiCast(_ipParameters.multicastAddress(), KNXIP_MULTICAST_PORT);
        _enabled = true;
        return;
    }

    if(!value && _enabled)
    {
        _platform.closeMultiCast();
        _enabled = false;
        return;
    }
}

bool DataLinkLayer::enabled() const
{
    return _enabled;
}


bool DataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    if (!_enabled)
        return false;

#ifdef DUMP_TELEGRAMS_
    {
        print.print("QSD: ");
        for (uint32_t i = 0; i <= length; ++i)
        {
            if (i) print.print(" ");
            print.print(bytes[i], HEX, 2);
        }
        print.println();
    }
#endif

    return _platform.sendBytes(bytes, length);
}