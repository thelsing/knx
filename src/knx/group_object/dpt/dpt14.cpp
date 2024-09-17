#include "dpt14.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt14::size() const
{
    return Go_4_Octets;
}

void Knx::Dpt14::encode(uint8_t* data) const
{
    float32ToPayload(data, 0, _value, 0xFFFFFFFF);
}

bool Knx::Dpt14::decode(uint8_t* data)
{
    _value = float32FromPayload(data, 0);
    return true;
}
