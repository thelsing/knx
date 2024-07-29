#include "cemi_frame.h"
#include "bits.h"
#include "string.h"
#include <stdio.h>

/*
cEMI Frame Format

              +--------+--------+--------+--------+---------+---------+--------+---------+
	          |                               _data                                      |
              +--------+--------+--------+--------+---------+---------+--------+---------+
	          |                               LPDU                                       |
			  +--------+--------+--------+--------+---------+---------+--------+---------+
	                                                                  |           NPDU   |
    +---------+--------+--------+--------+--------+---------+---------+--------+---------+
    | Header  |  Msg   |Add.Info| Ctrl 1 | Ctrl 2 | Source  | Dest.   |  Data  |   TPDU  |
    |         | Code   | Length |        |        | Address | Address | Length |   APDU  |
    +---------+--------+--------+--------+--------+---------+---------+--------+---------+
      6 bytes   1 byte   1 byte   1 byte   1 byte   2 bytes   2 bytes   1 byte   n bytes
 
        Header          = See below the structure of a cEMI header
        Message Code    = See below. On Appendix A is the list of all existing EMI and cEMI codes
        Add.Info Length = 0x00 - no additional info
        Control Field 1 =
        Control Field 2 =
        Source Address  = 0x0000 - filled in by router/gateway with its source address which is
                          part of the KNX subnet
        Dest. Address   = KNX group or individual address (2 byte)
        Data Length     = Number of bytes of data in the APDU excluding the TPCI/APCI bits
        APDU            = Application Protocol Data Unit - the actual payload including transport
                          protocol control information (TPCI), application protocol control
                          information (APCI) and data passed as an argument from higher layers of
                          the KNX communication stack
 
Control Field 1
 
          Bit  |
         ------+---------------------------------------------------------------
           7   | Frame Type  - 0x0 for extended frame
               |               0x1 for standard frame
         ------+---------------------------------------------------------------
           6   | Reserved
               |
         ------+---------------------------------------------------------------
           5   | Repeat Flag - 0x0 repeat frame on medium in case of an error
               |               0x1 do not repeat
         ------+---------------------------------------------------------------
           4   | System Broadcast - 0x0 system broadcast
               |                    0x1 broadcast
         ------+---------------------------------------------------------------
           3   | Priority    - 0x0 system
               |               0x1 normal
         ------+               0x2 urgent
           2   |               0x3 low
               |
         ------+---------------------------------------------------------------
           1   | Acknowledge Request - 0x0 no ACK requested
               | (L_Data.req)          0x1 ACK requested
         ------+---------------------------------------------------------------
           0   | Confirm      - 0x0 no error
               | (L_Data.con) - 0x1 error
         ------+---------------------------------------------------------------
 
         Control Field 2
 
          Bit  |
         ------+---------------------------------------------------------------
           7   | Destination Address Type - 0x0 individual address
               |                          - 0x1 group address
         ------+---------------------------------------------------------------
          6-4  | Hop Count (0-7)
         ------+---------------------------------------------------------------
          3-0  | Extended Frame Format - 0x0 standard frame
         ------+---------------------------------------------------------------
*/ 

CemiFrame::CemiFrame(uint8_t* data, uint16_t length)
    : _npdu(data + data[1] + NPDU_LPDU_DIFF, *this), 
      _tpdu(data + data[1] + TPDU_LPDU_DIFF, *this), 
      _apdu(data + data[1] + APDU_LPDU_DIFF, *this)
{
    _data = data;
    _ctrl1 = data + data[1] + CEMI_HEADER_SIZE;
    _length = length;
}

CemiFrame::CemiFrame(uint8_t apduLength)
    : _data(buffer),
      _npdu(_data + NPDU_LPDU_DIFF, *this), 
      _tpdu(_data + TPDU_LPDU_DIFF, *this), 
      _apdu(_data + APDU_LPDU_DIFF, *this)
{
    _ctrl1 = _data + CEMI_HEADER_SIZE;

    memset(_data, 0, apduLength + APDU_LPDU_DIFF);
    _ctrl1[0] |= Broadcast;
    _npdu.octetCount(apduLength);
    _length = _npdu.length() + NPDU_LPDU_DIFF;
}

CemiFrame::CemiFrame(const CemiFrame & other)
    : _data(buffer),
      _npdu(_data + NPDU_LPDU_DIFF, *this),
      _tpdu(_data + TPDU_LPDU_DIFF, *this),
      _apdu(_data + APDU_LPDU_DIFF, *this)
{
    _ctrl1 = _data + CEMI_HEADER_SIZE; 
    _length = other._length;

    memcpy(_data, other._data, other.totalLenght());
}

CemiFrame& CemiFrame::operator=(CemiFrame other)
{
    _length = other._length;
    _data = buffer;
    _ctrl1 = _data + CEMI_HEADER_SIZE;
    memcpy(_data, other._data, other.totalLenght());
    _npdu._data = _data + NPDU_LPDU_DIFF;
    _tpdu._data = _data + TPDU_LPDU_DIFF;
    _apdu._data = _data + APDU_LPDU_DIFF;
    return *this;
}


MessageCode CemiFrame::messageCode() const
{
    return (MessageCode)_data[0];
}

void CemiFrame::messageCode(MessageCode msgCode)
{
    _data[0] = msgCode;
}

uint16_t CemiFrame::totalLenght() const
{
    return _length;
}

uint16_t CemiFrame::telegramLengthtTP() const
{
    if (frameType() == StandardFrame)
        return totalLenght() - 2; /*-AddInfo -MsgCode - only one CTRL + CRC, */
    else
        return totalLenght() - 1; /*-AddInfo -MsgCode + CRC, */
}

void CemiFrame::fillTelegramTP(uint8_t* data)
{
    uint16_t len = telegramLengthtTP();
    
    if (frameType() == StandardFrame)
    {
        uint8_t octet5 = (_ctrl1[1] & 0xF0) | (_ctrl1[6] & 0x0F);
        data[0] = _ctrl1[0]; //CTRL
        memcpy(data + 1, _ctrl1 + 2, 4); // SA, DA
        data[5] = octet5; // LEN; Hopcount, ..
        memcpy(data + 6, _ctrl1 + 7, len - 7); // APDU
    }
    else
    {
        memcpy(data, _ctrl1, len - 1);
    }
    data[len - 1] = calcCrcTP(data, len - 1);
}

#ifdef USE_RF

uint16_t CemiFrame::telegramLengthtRF() const
{
    return totalLenght() - 3;
}

void CemiFrame::fillTelegramRF(uint8_t* data)
{
    uint16_t len = telegramLengthtRF();

    // We prepare the actual KNX telegram for RF here only.
    // The packaging into blocks with CRC16 (Format based on FT3 Data Link Layer (IEC 870-5))
    // is done in the RF Data Link Layer code.
    // RF always uses the Extended Frame Format. However, the length field is missing (right before the APDU)
    // as there is already a length field at the beginning of the raw RF frame which is also used by the 
    // physical layer to control the HW packet engine of the transceiver.

    data[0] = _ctrl1[1] & 0x0F; // KNX CTRL field for RF (bits 3..0 EFF only), bits 7..4 are set to 0 for asynchronous RF frames
    memcpy(data + 1, _ctrl1 + 2, 4); // SA, DA
    data[5] = (_ctrl1[1] & 0xF0) | ((_rfLfn & 0x7) << 1) | ((_ctrl1[0] & 0x10) >> 4); // L/NPCI field: AT, Hopcount, LFN, AET
    memcpy(data + 6, _ctrl1 + 7, len - 6); // APDU

    //printHex("cEMI_fill: ", &data[0], len);
}
#endif
uint8_t* CemiFrame::data()
{
    return _data;
}

uint16_t CemiFrame::dataLength()
{
    return _length;
}

uint8_t CemiFrame::calcCrcTP(uint8_t * buffer, uint16_t len)
{
    uint8_t crc = 0xFF;
    
    for (uint16_t i = 0; i < len; i++)
        crc ^= buffer[i];
    
    return crc;
}

FrameFormat CemiFrame::frameType() const
{
    return (FrameFormat)(_ctrl1[0] & StandardFrame);
}

void CemiFrame::frameType(FrameFormat type)
{
    _ctrl1[0] &= ~StandardFrame;
    _ctrl1[0] |= type;
}

Repetition CemiFrame::repetition() const
{
    return (Repetition)(_ctrl1[0] & RepetitionAllowed);
}

void CemiFrame::repetition(Repetition rep)
{
    _ctrl1[0] &= ~RepetitionAllowed;
    _ctrl1[0] |= rep;
}

SystemBroadcast CemiFrame::systemBroadcast() const
{
    return (SystemBroadcast)(_ctrl1[0] & Broadcast);
}

void CemiFrame::systemBroadcast(SystemBroadcast value)
{
    _ctrl1[0] &= ~Broadcast;
    _ctrl1[0] |= value;
}

Priority CemiFrame::priority() const
{
    return (Priority)(_ctrl1[0] & LowPriority);
}

void CemiFrame::priority(Priority value)
{
    _ctrl1[0] &= ~LowPriority;
    _ctrl1[0] |= value;
}

AckType CemiFrame::ack() const
{
    return (AckType)(_ctrl1[0] & AckRequested);
}

void CemiFrame::ack(AckType value)
{
    _ctrl1[0] &= ~AckRequested;
    _ctrl1[0] |= value;
}

Confirm CemiFrame::confirm() const
{
    return (Confirm)(_ctrl1[0] & ConfirmError);
}

void CemiFrame::confirm(Confirm value)
{
    _ctrl1[0] &= ~ConfirmError;
    _ctrl1[0] |= value;
}

AddressType CemiFrame::addressType() const
{
    return (AddressType)(_ctrl1[1] & GroupAddress);
}

void CemiFrame::addressType(AddressType value)
{
    _ctrl1[1] &= ~GroupAddress;
    _ctrl1[1] |= value;
}

uint8_t CemiFrame::hopCount() const
{
    return ((_ctrl1[1] >> 4) & 0x7);
}

void CemiFrame::hopCount(uint8_t value)
{
    _ctrl1[1] &= ~(0x7 << 4);
    _ctrl1[1] |= ((value & 0x7) << 4);
}

uint16_t CemiFrame::sourceAddress() const
{
    uint16_t addr;
    popWord(addr, _ctrl1 + 2);
    return addr;
}

void CemiFrame::sourceAddress(uint16_t value)
{
    pushWord(value, _ctrl1 + 2);
}

uint16_t CemiFrame::destinationAddress() const
{
    uint16_t addr;
    popWord(addr, _ctrl1 + 4);
    return addr;
}

void CemiFrame::destinationAddress(uint16_t value)
{
    pushWord(value, _ctrl1 + 4);
}
#ifdef USE_RF
uint8_t* CemiFrame::rfSerialOrDoA() const
{
    return _rfSerialOrDoA;
}

void CemiFrame::rfSerialOrDoA(const uint8_t* rfSerialOrDoA)
{
    _rfSerialOrDoA = (uint8_t*)rfSerialOrDoA;
}

uint8_t CemiFrame::rfInfo() const
{
    return _rfInfo;
}

void CemiFrame::rfInfo(uint8_t rfInfo)
{
    _rfInfo = rfInfo;
}

uint8_t CemiFrame::rfLfn() const
{
    return _rfLfn;
}

void CemiFrame::rfLfn(uint8_t rfLfn)
{
    _rfLfn = rfLfn;
}
#endif
NPDU& CemiFrame::npdu()
{
    return _npdu;
}

TPDU& CemiFrame::tpdu()
{
    return _tpdu;
}

APDU& CemiFrame::apdu()
{
    return _apdu;
}

bool CemiFrame::valid() const
{
    uint8_t addInfoLen = _data[1];
    uint8_t apduLen = _data[_data[1] + NPDU_LPDU_DIFF];

    if (_length != 0 && _length != (addInfoLen + apduLen + NPDU_LPDU_DIFF + 2))
    {
        print("length issue, length: ");
        print(_length);
        print(" addInfoLen: ");
        print(addInfoLen);
        print(" apduLen: ");
        print(apduLen);
        print(" expected length: ");
        println(addInfoLen + apduLen + NPDU_LPDU_DIFF + 2);
        printHex("Frame: ", _data, _length, true);

        return false;
    }
    if ((_ctrl1[0] & 0x40) > 0 // Bit 6 has do be 0
        || (_ctrl1[1] & 0xF) > 0 // only standard or extended frames
        || _npdu.octetCount() == 0xFF // not allowed
        || (_npdu.octetCount() > 15 && frameType() == StandardFrame)
        ){
        print("Other issue");
        return false;
        }

    return true;
}
