#include "dpt2.h"

#include "dptconvert.h"

Knx::Dpt2::Dpt2(unsigned short subgroup /* = 0*/) : Dpt(2, subgroup) {}

Knx::Dpt2::Dpt2(Dpt2Value value) : Dpt2()
{
    _value = value;
}

Knx::Go_SizeCode Knx::Dpt2::size() const
{
    return Go_2_Bit;
}

void Knx::Dpt2::encode(uint8_t* data) const
{
    if (_value ==  NoControl)
    {
        bitToPayload(data, 6, false);
        return;
    }

    bitToPayload(data, 6, true);
    bitToPayload(data, 7, _value == Control_Function1);
}

void Knx::Dpt2::decode(uint8_t* data)
{
    bool c = bitFromPayload(data, 6);

    if (!c)
    {
        _value =  NoControl;
        return;
    }

    bool v = bitFromPayload(data, 7);

    _value = v ? Control_Function1 : Control_Function0;
}

void Knx::Dpt2::value(Dpt2Value value)
{
    _value = value;
}

Knx::Dpt2::Dpt2Value Knx::Dpt2::value() const
{
    return _value;
}

Knx::Dpt2::operator Dpt2Value() const
{
    return _value;
}


Knx::Dpt2& Knx::Dpt2::operator=(const Dpt2Value value)
{
    _value = value;
    return *this;
}
