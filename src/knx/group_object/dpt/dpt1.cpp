#include "dpt1.h"

#include "dptconvert.h"

Knx::Dpt1::Dpt1() {}

Knx::Dpt1::Dpt1(bool value)
{
    this->value(value);
}

Knx::Go_SizeCode Knx::Dpt1::size() const
{
    return Go_1_Bit;
}

void Knx::Dpt1::encode(uint8_t* data) const
{
    bitToPayload(data, 7, _value);
}

bool Knx::Dpt1::decode(uint8_t* data)
{
    _value = bitFromPayload(data, 7);
    return true;
}

void Knx::Dpt1::value(bool value)
{
    _value = value;
}

bool Knx::Dpt1::value() const
{
    return _value;
}

Knx::Dpt1::operator bool() const
{
    return _value;
}

Knx::Dpt1& Knx::Dpt1::operator=(const bool value)
{
    _value = value;
    return *this;
}
