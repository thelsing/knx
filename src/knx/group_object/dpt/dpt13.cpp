#include "dpt13.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt13::size() const
{
    return Go_4_Octets;
}

void Knx::Dpt13::encode(uint8_t* data) const
{
    signed32ToPayload(data, 0, _value, 0xFFFFFFFF);
}

bool Knx::Dpt13::decode(uint8_t* data)
{
    _value = signed32FromPayload(data, 0);
    return true;
}
