#include "dpt4.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt4::size() const
{
    return Go_1_Octet;
}

void Knx::Dpt4::encode(uint8_t* data) const
{
    unsigned8ToPayload(data, 0, value(), 0xFF);
}

bool Knx::Dpt4::decode(uint8_t* data)
{
    value(unsigned8FromPayload(data, 0));
    return true;
}

bool Knx::DPT_Char_ASCII::decode(uint8_t* data)
{
    Dpt4::value(unsigned8FromPayload(data, 0) & 0x7F);
    return true;
}

void Knx::DPT_Char_ASCII::value(char value)
{
    Dpt4::value(value & 0x7F);
}
