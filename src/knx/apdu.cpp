#include "apdu.h"
#include "cemi_frame.h"
#include "bits.h"

APDU::APDU(uint8_t* data, CemiFrame& frame): _data(data), _frame(frame)
{
}

ApduType APDU::type()
{
    uint16_t apci;
    apci = getWord(_data);
    popWord(apci, _data);
    apci &= 0x3ff;
    if ((apci >> 6) < 11 && (apci >> 6) != 7) //short apci
        apci &= 0x3c0;
    return (ApduType)apci;
}

void APDU::type(ApduType atype)
{
    // ApduType is in big endian so convert to host first, pushWord converts back
    pushWord((uint16_t)atype, _data);
}

uint8_t* APDU::data()
{
    return _data + 1;
}

CemiFrame& APDU::frame()
{
    return _frame;
}

uint8_t APDU::length() const
{
    return _frame.npdu().octetCount();
}

void APDU::printPDU()
{
    print("APDU: ");
    print(type(), HEX);
    print("  ");
    print(_data[0] & 0x3, HEX);
    for (uint8_t i = 1; i < length() + 1; ++i)
    {
        if (i) print(" ");
        print(_data[i], HEX);
    }
    println();
}
