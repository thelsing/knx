#include "dpt8.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt8::size() const
{
    return Go_2_Octets;
}

void Knx::Dpt8::encode(uint8_t* data) const
{
    signed16ToPayload(data, 0, _value, 0xFFFF);
}

bool Knx::Dpt8::decode(uint8_t* data)
{
    _value = signed16FromPayload(data, 0);
    return true;
}
