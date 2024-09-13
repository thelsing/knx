#include "dpt4.h"

#include "dptconvert.h"

Knx::Dpt4::Dpt4() {}

Knx::Dpt4::Dpt4(char value)
{
    _value = value;
}

Knx::Go_SizeCode Knx::Dpt4::size() const
{
    return Go_1_Octet;
}

void Knx::Dpt4::encode(uint8_t* data) const
{
    unsigned8ToPayload(data, 0, _value, 0xFF);
}

bool Knx::Dpt4::decode(uint8_t* data)
{
    _value = signed8FromPayload(data, 0);
    return true;
}

void Knx::Dpt4::value(char value)
{
    _value = value;
}

char Knx::Dpt4::value() const
{
    return _value;
}

Knx::Dpt4::operator char() const
{
    return _value;
}

Knx::Dpt4& Knx::Dpt4::operator=(const char value)
{
    _value = value;
    return *this;
}

bool Knx::DPT_Char_ASCII::decode(uint8_t* data)
{
    Dpt4::value(signed8FromPayload(data, 0) & 0x7F);
    return true;
}

void Knx::DPT_Char_ASCII::value(char value)
{
    Dpt4::value(value & 0x7F);
}
