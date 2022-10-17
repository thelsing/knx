#include "config.h"
#ifdef USE_RF

#if defined(DeviceFamily_CC13X0)
  #include "rf_physical_layer_cc1310.h"
#else
  #include "rf_physical_layer_cc1101.h"
#endif
#include "rf_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "rf_medium_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

void RfDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    _rfPhy.loop();        
}

bool RfDataLinkLayer::sendFrame(CemiFrame& frame)
{
    // If no serial number of domain address was set,
    // use our own SN/DoA
    if (frame.rfSerialOrDoA() == nullptr)
    {
        // Depending on this flag, use either KNX Serial Number
        // or the RF domain address that was programmed by ETS
        if (frame.systemBroadcast() == SysBroadcast)
        {
            frame.rfSerialOrDoA((uint8_t*)_deviceObject.propertyData(PID_SERIAL_NUMBER));
        }
        else
        {
            frame.rfSerialOrDoA(_rfMediumObj.rfDomainAddress());
        }
    }

    // If Link Layer frame is set to 0xFF,
    // use our own counter
    if (frame.rfLfn() == 0xFF)
    {
        // Set Data Link Layer Frame Number
        frame.rfLfn(_frameNumber);
        // Link Layer frame number counts 0..7
        _frameNumber = (_frameNumber + 1) & 0x7;
    }

    // bidirectional device, battery is ok, signal strength indication is void (no measurement)
    frame.rfInfo(0x02);

    if (!_enabled)
    {
        dataConReceived(frame, false);
        return false;
    }

    // TODO: Is queueing really required?
    // According to the spec. the upper layer may only send a new L_Data.req if it received
    // the L_Data.con for the previous L_Data.req.
    addFrameTxQueue(frame);

    // TODO: For now L_data.req is confirmed immediately (L_Data.con)
    // see 3.6.3 p.80: L_Data.con shall be generated AFTER transmission of the corresponsing frame
    // RF sender will never generate L_Data.con with C=1 (Error), but only if the TX buffer overflows
    // The RF sender cannot detect if the RF frame was transmitted successfully or not according to the spec.
    dataConReceived(frame, true);

    return true;
}

RfDataLinkLayer::RfDataLinkLayer(DeviceObject& devObj, RfMediumObject& rfMediumObj,
                                         NetworkLayerEntity &netLayerEntity, Platform& platform)
    : DataLinkLayer(devObj, netLayerEntity, platform),
      _rfMediumObj(rfMediumObj),
      _rfPhy(*this, platform)
{
}

void RfDataLinkLayer::frameBytesReceived(uint8_t* rfPacketBuf, uint16_t length)
{
    // RF data link layer frame format
    // See 3.2.5 p.22

    // First block + smallest KNX telegram will give a minimum size of 22 bytes with checksum bytes
    if (length < 21)
    {
        print("Received packet is too small. length: ");
        println(length);
        return;
    }

#if defined(DeviceFamily_CC13X0)
        // Small optimization:
        // We do not calculate the CRC16-DNP again for the first block.
        // It was already done in the CC13x0 RX driver during reception.
        // Also the two fixed bytes 0x44 and 0xFF are also there.
        // So if we get here we can assume a valid block 1
#else        
    // CRC16-DNP of first block is always located here
    uint16_t block1Crc = rfPacketBuf[10] << 8 | rfPacketBuf[11];
    
    // If the checksum was ok and the other
    // two constant header bytes match the KNX-RF spec. (C-field: 0x44 and ESC-field: 0xFF)...
    // then we seem to have a valid first block of an KNX RF frame.
    // The first block basically contains the RF-info field and the KNX SN/Domain address.
    if ((rfPacketBuf[1] == 0x44) &&
        (rfPacketBuf[2] == 0xFF) &&
        (crc16Dnp(rfPacketBuf, 10) == block1Crc))
#endif        
    {
        // bytes left from the remaining block(s)
        uint16_t bytesLeft = length - 12;
        // we use two pointers to move over the two buffers
        uint8_t* pRfPacketBuf = &rfPacketBuf[12]; // pointer to start of RF frame block 2 (with CTRL field)
        // Reserve 1 byte (+1) for the second ctrl field 
        // cEMI frame has two CTRL fields, but RF frame has only one, but uses ALWAYS extended frames
        // Information for BOTH cEMI CTRL fields is distributed in a RF frame (RF CTRL field and RF L/NPCI field)
        // So we cannot just copy an RF frame with CTRL fields as is
        // KNX RF frame will be placed starting at cEMI CTRL2 field (so RF CTRL field is CTRL2 field cEMI)
        uint8_t* pBuffer = &_buffer[CEMI_HEADER_SIZE + 1]; 
        // New length of the packet with CRC bytes removed, add space for CEMI header and the second CTRL field
        uint16_t newLength = CEMI_HEADER_SIZE + 1;

        // Now check each block checksum and copy the payload of the block
        // into a new buffer without checksum
        uint16_t blockCrc;
        bool crcOk = true;
        while (bytesLeft > 18)
        {
            // Get CRC16 from end of the block
            blockCrc = pRfPacketBuf[16] << 8 | pRfPacketBuf[17];
            if (crc16Dnp(pRfPacketBuf, 16) == blockCrc)
            {
                // Copy only the payload without the checksums
                memcpy(pBuffer, pRfPacketBuf, 16);
            }
            else
            {
                crcOk = false;
                break;
            }
            pBuffer += 16;
            pRfPacketBuf += 18;
            newLength += 16;
            bytesLeft -= 18;
        }

        // Now process the last block
        blockCrc = pRfPacketBuf[bytesLeft - 2] << 8 | pRfPacketBuf[bytesLeft - 1];
        crcOk = crcOk && (crc16Dnp(&pRfPacketBuf[0], bytesLeft -2) == blockCrc);

        // If all checksums were ok, then...
        if (crcOk)
        {
            // Copy rest of the received packet without checksum
            memcpy(pBuffer, pRfPacketBuf,  bytesLeft -2);
            newLength += bytesLeft -2;

            // Prepare CEMI by writing/overwriting certain fields in the buffer (contiguous frame without CRC checksums)
            // See 3.6.3 p.79: L_Data services for KNX RF asynchronous frames 
            // For now we do not use additional info, but use normal method arguments for CEMI
            _buffer[0] = (uint8_t) L_data_ind;  // L_data.ind
            _buffer[1] = 0;     // Additional info length (spec. says that local dev management is not required to use AddInfo internally)
            _buffer[2] = 0;     // CTRL1 field (will be set later, this is the field we reserved space for)
            _buffer[3] &= 0x0F; // CTRL2 field (take only RFCtrl.b3..0, b7..4 shall always be 0 for asynchronous KNX RF)

            // Now get all control bits from the L/NPCI field of the RF frame 
            // so that we can overwrite it afterwards with the correct NPDU length
            // Get data link layer frame number (LFN field) from L/NPCI.LFN (bit 3..1)
            uint8_t lfn = (_buffer[8] & 0x0E) >> 1;
            // Get address type from L/NPCI.LFN (bit 7)
            AddressType addressType = (_buffer[8] & 0x80) ? GroupAddress:IndividualAddress;
            // Get routing counter from L/NPCI.LFN (bit 6..4) and map to hop count in Ctrl2.b6-4
            uint8_t hopCount = (_buffer[8] & 0x70) >> 4;
            // Get AddrExtensionType from L/NPCI.LFN (bit 7) and map to system broadcast flag in Ctrl1.b4
            SystemBroadcast systemBroadcast = (_buffer[8] & 0x01) ? Broadcast:SysBroadcast;

            // Setup L field of the cEMI frame with the NPDU length
            // newLength -8 bytes (NPDU_LPDU_DIFF, no AddInfo) -1 byte length field -1 byte TPCI/APCI bits
            _buffer[8] = newLength - NPDU_LPDU_DIFF - 1 - 1;

            // If we have a broadcast message (within the domain),
            // then we received the domain address and not the KNX serial number
            if (systemBroadcast == Broadcast)
            {
                // Check if the received RF domain address matches the one stored in the RF medium object
                // If it does not match then skip the remaining processing
                if (memcmp(_rfMediumObj.rfDomainAddress(), &rfPacketBuf[4], 6))
                {
                    println("RX domain address does not match. Skipping...");
                    return;
                }
            }

            // TODO
            // Frame duplication prevention based on LFN (see KKNX RF spec. 3.2.5 p.28)

            // Prepare the cEMI frame
            CemiFrame frame(_buffer, newLength);
            frame.frameType(ExtendedFrame);         // KNX RF uses only extended frame format
            frame.priority(SystemPriority);         // Not used in KNX RF
            frame.ack(AckDontCare);                 // Not used in KNX RF 
            frame.systemBroadcast(systemBroadcast); // Mapped from flag AddrExtensionType (KNX serial(0) or Domain Address(1))
            frame.hopCount(hopCount);               // Hop count from routing counter
            frame.addressType(addressType);         // Group address or individual address
            frame.rfSerialOrDoA(&rfPacketBuf[4]);   // Copy pointer to field Serial or Domain Address (check broadcast flag what it is exactly)
            frame.rfInfo(rfPacketBuf[3]);           // RF-info field (1 byte)
            frame.rfLfn(lfn);                       // Data link layer frame number (LFN field)
/*
            print("RX LFN: ");
            print(lfn);
            print(" len: ");
            print(newLength);

            print(" data: ");
            printHex(" data: ", _buffer, newLength);
*/
            frameReceived(frame);
        }
    }
}

void RfDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        if (_rfPhy.InitChip())
        {
            _enabled = true;
            print("ownaddr ");
            println(_deviceObject.individualAddress(), HEX);
        }
        else
        {
        	_enabled = false;
            println("ERROR, RF transceiver not responding");
        }
        return;
    }

    if (!value && _enabled)
    {
        _rfPhy.stopChip();
        _enabled = false;
        return;
    }
}

bool RfDataLinkLayer::enabled() const
{
    return _enabled;
}

DptMedium RfDataLinkLayer::mediumType() const
{
    return DptMedium::KNX_RF;
}

void RfDataLinkLayer::fillRfFrame(CemiFrame& frame, uint8_t* data)
{
    uint16_t crc;
    uint16_t length = frame.telegramLengthtRF();

    data[0] = 9 + length;     // Length block1 (always 9 bytes, without length itself) + Length of KNX telegram without CRCs
    data[1] = 0x44;           // C field: According to IEC870-5. KNX only uses SEND/NO REPLY (C = 44h) 
    data[2] = 0xFF;           // ESC field: This field shall have the fixed value FFh. 
    data[3] = frame.rfInfo(); // RF-info field

    // Generate CRC16-DNP over the first block of data
    pushByteArray(frame.rfSerialOrDoA(), 6, &data[4]);
    crc = crc16Dnp(&data[0], 10);
    pushWord(crc, &data[10]);

    // Put the complete KNX telegram into a temporary buffer
    // as we have to add CRC16 checksums after each block of 16 bytes
    frame.fillTelegramRF(_buffer);

    // Create a checksum for each block of full 16 bytes
    uint16_t bytesLeft = length;
    uint8_t *pBuffer = &_buffer[0];
    uint8_t *pData = &data[12];
    while (bytesLeft > 16)
    {
        memcpy(pData, pBuffer, 16);
        crc = crc16Dnp(pData, 16);
        pushWord(crc, &pData[16]);

        pBuffer += 16;
        pData += 18;
        bytesLeft -= 16;
    }

    // Copy remaining bytes of last block. Could be less than 16 bytes
    memcpy(pData, pBuffer, bytesLeft);
    // And add last CRC
    crc = crc16Dnp(pData, bytesLeft);
    pushWord(crc, &pData[bytesLeft]);
}

void RfDataLinkLayer::addFrameTxQueue(CemiFrame& frame)
{
    _tx_queue_frame_t* tx_frame = new _tx_queue_frame_t;

    uint16_t length = frame.telegramLengthtRF(); // Just the pure KNX telegram from CTRL field until end of APDU
    uint8_t nrFullBlocks = length / 16;          // Number of full (16 bytes) RF blocks required
    uint8_t bytesLeft = length % 16;             // Remaining bytes of the last packet

    // Calculate total number of bytes required to store the complete raw RF frame
    // Block1 always requires 12 bytes including Length and CRC
    // Each full block has 16 bytes payload plus 2 bytes CRC
    // Add remaining bytes of the last block and add 2 bytes for CRC
    uint16_t totalLength = 12 + (nrFullBlocks * 18) + bytesLeft + 2;

    tx_frame->length = totalLength;
    tx_frame->data = new uint8_t[tx_frame->length];
    tx_frame->next = NULL;

    // Prepare the raw RF frame
    fillRfFrame(frame, tx_frame->data);
/*
    print("TX LFN: ");
    print(frame.rfLfn());

    print(" len: ");
    print(totalLength);

    printHex(" data:", tx_frame->data, totalLength);
*/
    if (_tx_queue.back == NULL)
    {
        _tx_queue.front = _tx_queue.back = tx_frame;
    }
    else
    {
        _tx_queue.back->next = tx_frame;
        _tx_queue.back = tx_frame;
    }
}

bool RfDataLinkLayer::isTxQueueEmpty()
{
    if (_tx_queue.front == NULL)
    {
        return true;
    }
    return false;
}

void RfDataLinkLayer::loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength)
{
    if (_tx_queue.front == NULL)
    {
        return;
    }
    _tx_queue_frame_t* tx_frame = _tx_queue.front;
    *sendBuffer = tx_frame->data;
    *sendBufferLength = tx_frame->length;
    _tx_queue.front = tx_frame->next;

    if (_tx_queue.front == NULL)
    {
        _tx_queue.back = NULL;
    }
    delete tx_frame;
}

#endif
