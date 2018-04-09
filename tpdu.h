#pragma once

#include "stdint.h"
#include "knx_types.h"
class CemiFrame;
class APDU;

class TPDU
{
    friend class CemiFrame;
public:
    TPDU(uint8_t* data, CemiFrame& frame);
    TpduType type() const;
    void type(TpduType type);

    bool numbered() const;
    void numbered(bool value);

    bool control() const;
    void control(bool value);

    uint8_t sequenceNumber() const;
    void sequenceNumber(uint8_t value);

    APDU& apdu();

    CemiFrame& frame();
    void printPDU();
private:
    uint8_t* _data;
    CemiFrame& _frame;
};
