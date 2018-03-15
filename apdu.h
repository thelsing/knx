#pragma once

#include <stdint.h>
#include "knx_types.h"

class CemiFrame;

class APDU
{
    friend class CemiFrame;
public:
    APDU(uint8_t* data, CemiFrame& frame);
    ApduType type();
    void type(ApduType atype);
    uint8_t* data();
    CemiFrame& frame();
    uint8_t length() const;
    void printPDU();
private:
    uint8_t* _data;
    CemiFrame& _frame;
};