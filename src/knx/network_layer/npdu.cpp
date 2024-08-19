#include "npdu.h"

#include "../datalink_layer/cemi_frame.h"
#include "../bits.h"

#include <cstring>

NPDU::NPDU(uint8_t* data, CemiFrame& frame): _data(data), _frame(frame)
{
}


uint8_t NPDU::octetCount() const
{
    return _data[0];
}

void NPDU::octetCount(uint8_t value)
{
    _data[0] = value;
}

uint8_t NPDU::length() const
{
    return _data[0] + 2; // +1 for length field, +1 for TCPI
}

uint8_t NPDU::hopCount() const
{
    return _frame.hopCount();
}

void NPDU::hopCount(uint8_t value)
{
    _frame.hopCount(value);
}

CemiFrame& NPDU::frame()
{
    return _frame;
}

TPDU& NPDU::tpdu()
{
    return _frame.tpdu();
}

void NPDU::printIt() const
{
#ifndef KNX_NO_PRINT
    print("NPDU: Octetcount: ");
    print(octetCount());
    print(" hopCount ");
    print(hopCount());
#endif
}