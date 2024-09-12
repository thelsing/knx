#include "dpt9.h"

#include "dptconvert.h"

Knx::Dpt9::Dpt9() {}

Knx::Dpt9::Dpt9(float value) : _value(value) {}

Knx::Go_SizeCode Knx::Dpt9::size() const
{
    return Go_1_Bit;
}

void Knx::Dpt9::encode(uint8_t* data) const
{
    float16ToPayload(data, 0, _value, 0xFFFF);
}

void Knx::Dpt9::decode(uint8_t* data)
{
    if (unsigned16FromPayload(data, 0) == 0x7FFF)
        return;

    _value = float16FromPayload(data, 0);
}

void Knx::Dpt9::value(float value)
{
    _value = value;
}

bool Knx::Dpt9::value()
{
    return _value;
}

Knx::Dpt9::operator float() const
{
    return _value;
}


Knx::Dpt9& Knx::Dpt9::operator=(const float value)
{
    _value = value;
    return *this;
}
