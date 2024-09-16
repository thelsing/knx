#include "dpt12.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt12::size() const
{
    return Go_4_Octets;
}

void Knx::Dpt12::encode(uint8_t* data) const
{
    unsigned32ToPayload(data, 0, _value, 0xFFFFFFFF);
}

bool Knx::Dpt12::decode(uint8_t* data)
{
    _value = unsigned32FromPayload(data, 0);
    return true;
}
