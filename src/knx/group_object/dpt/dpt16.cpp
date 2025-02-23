#include "dpt16.h"

#include "dptconvert.h"
#include <cstring>

Knx::Dpt16::Dpt16()
{
    memset(_value, 0, 15);
}

Knx::Dpt16::Dpt16(const char* value)
    : Dpt16()
{
    this->value(value);
}

Knx::Go_SizeCode Knx::Dpt16::size() const
{
    return Go_14_Octets;
}

void Knx::Dpt16::encode(uint8_t* data) const
{
    uint8_t val = _value[0];

    for (int n = 0; n < 14; n++)
    {
        if (val)
            val = _value[n]; // string terminator 0x00 will stop further assignments and init the remainig payload with zero

        unsigned8ToPayload(data, n, val, 0xff);
    }
}

bool Knx::Dpt16::decode(uint8_t* data)
{

    _value[14] = '\0';

    for (int n = 0; n < 14; ++n)
    {
        _value[n] = signed8FromPayload(data, n);
    }

    return true;
}

const char* Knx::Dpt16::value() const
{
    return _value;
}

void Knx::Dpt16::value(const char* value)
{
    strncpy(_value, value, 14);
    _value[14] = 0;
}

Knx::DPT_String_ASCII::DPT_String_ASCII()
    : Dpt16()
{}

Knx::DPT_String_ASCII::DPT_String_ASCII(const char* value)
    : Dpt16(value)
{}

bool Knx::DPT_String_ASCII::decode(uint8_t* data)
{
    _value[14] = '\0';

    for (int n = 0; n < 14; ++n)
    {
        _value[n] = signed8FromPayload(data, n);

        if ((_value[n] & 0x80))
        {
            _value[0] = 0;
            return false;
        }
    }

    return true;
}
