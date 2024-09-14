#include "dpt7.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt7::size() const
{
    return Go_2_Octets;
}

void Knx::Dpt7::encode(uint8_t* data) const
{
    unsigned16ToPayload(data, 0, _value, 0xFFFF);
}

bool Knx::Dpt7::decode(uint8_t* data)
{
    _value = unsigned16FromPayload(data, 0);
    return true;
}
