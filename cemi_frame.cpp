#include "cemi_frame.h"
#include "bits.h"
#include "string.h"
#include <stdio.h>

CemiFrame::CemiFrame(uint8_t* data, uint16_t length): _npdu(data + NPDU_LPDU_DIFF, *this), 
    _tpdu(data + TPDU_LPDU_DIFF, *this), _apdu(data + APDU_LPDU_DIFF, *this)
{
    _data = data;
    _ctrl1 = data + data[1] + 2;
    _length = length;
}

CemiFrame::CemiFrame(uint8_t apduLength): _data(buffer),
    _npdu(_data + NPDU_LPDU_DIFF, *this), _tpdu(_data + TPDU_LPDU_DIFF, *this), _apdu(_data + APDU_LPDU_DIFF, *this)
{
    _ctrl1 = _data + 2;
    _length = 0;

    memset(_data, 0, apduLength + APDU_LPDU_DIFF);
    _ctrl1[0] |= Broadcast;
    _npdu.octetCount(apduLength);
}

CemiFrame::CemiFrame(const CemiFrame & other): _data(buffer),
    _npdu(_data + NPDU_LPDU_DIFF, *this), _tpdu(_data + TPDU_LPDU_DIFF, *this), _apdu(_data + APDU_LPDU_DIFF, *this)
{
    _ctrl1 = _data + 2;
    _length = other._length;

    memcpy(_data, other._data, other.totalLenght());
}

CemiFrame& CemiFrame::operator=(CemiFrame other)
{
    _length = other._length;
    _data = buffer;
    _ctrl1 = _data + 2;
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
    uint16_t tmp = 
    _npdu.length() + NPDU_LPDU_DIFF;
    return tmp;
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
        memcpy(data + 6, _ctrl1 + 8, len - 7); // APDU
    }
    else
    {
        memcpy(data, _ctrl1, len - 1);
    }
    data[len - 1] = calcCRC(data, len - 1);
}

uint8_t CemiFrame::calcCRC(uint8_t * buffer, uint16_t len)
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
    return (Repetition)(_ctrl1[0] & RepititionAllowed);
}

void CemiFrame::repetition(Repetition rep)
{
    _ctrl1[0] &= ~RepititionAllowed;
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
    uint8_t apduLen = _data[1 + _data[1] + NPDU_LPDU_DIFF];

    if (_length != 0 && _length != (addInfoLen + apduLen + NPDU_LPDU_DIFF + 2))
        return false;
    
    if ((_ctrl1[0] & 0x40) > 0 // Bit 6 has do be 0
        || (_ctrl1[1] & 0xF) > 0 // only standard or extended frames
        || _npdu.octetCount() == 0xFF // not allowed
        || (_npdu.octetCount() > 15 && frameType() == StandardFrame)
        )
        return false;

    return true;
}
