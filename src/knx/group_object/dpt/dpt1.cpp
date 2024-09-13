#include "dpt1.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt1::size() const
{
    return Go_1_Bit;
}

void Knx::Dpt1::encode(uint8_t* data) const
{
    bitToPayload(data, 7, value());
}

bool Knx::Dpt1::decode(uint8_t* data)
{
    value(bitFromPayload(data, 7));
    return true;
}
