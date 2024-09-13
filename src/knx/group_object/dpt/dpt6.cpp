#include "dpt6.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt6::size() const
{
    return Go_1_Octet;
}

void Knx::Dpt6::encode(uint8_t* data) const
{
    signed8ToPayload(data, 0, value(), 0xFF);
}

bool Knx::Dpt6::decode(uint8_t* data)
{
    value(signed8FromPayload(data, 0));
    return true;
}

bool Knx::DPT_Status_Mode3::decode(uint8_t* data)
{
    int8_t val = signed8FromPayload(data, 0);

    int8_t mode = val & 0x7;

    if ( mode != (1 << 0) && mode != (1 << 1) && mode != (1 << 2) )
        return false;

    value(val);
    return true;
}

Knx::DPT_Status_Mode3::SetClearValue Knx::DPT_Status_Mode3::A()
{
    return ( _value & (1 << 7) ) > 0 ? Clear : Set;
}

void Knx::DPT_Status_Mode3::A(SetClearValue value)
{
    if (value == Set)
        _value &= ~ (1 << 7);
    else
        _value |= (1 << 7);
}


Knx::DPT_Status_Mode3::SetClearValue Knx::DPT_Status_Mode3::B()
{
    return ( _value & (1 << 6) ) > 0 ? Clear : Set;
}

void Knx::DPT_Status_Mode3::B(SetClearValue value)
{
    if (value == Set)
        _value &= ~ (1 << 6);
    else
        _value |= (1 << 6);
}

Knx::DPT_Status_Mode3::SetClearValue Knx::DPT_Status_Mode3::C()
{
    return ( _value & (1 << 5) ) > 0 ? Clear : Set;
}

void Knx::DPT_Status_Mode3::C(SetClearValue value)
{
    if (value == Set)
        _value &= ~ (1 << 5);
    else
        _value |= (1 << 5);
}

Knx::DPT_Status_Mode3::SetClearValue Knx::DPT_Status_Mode3::D()
{
    return ( _value & (1 << 4) ) > 0 ? Clear : Set;
}

void Knx::DPT_Status_Mode3::D(SetClearValue value)
{
    if (value == Set)
        _value &= ~ (1 << 4);
    else
        _value |= (1 << 4);
}

Knx::DPT_Status_Mode3::SetClearValue Knx::DPT_Status_Mode3::E()
{
    return ( _value & (1 << 3) ) > 0 ? Clear : Set;
}

void Knx::DPT_Status_Mode3::E(SetClearValue value)
{
    if (value == Set)
        _value &= ~ (1 << 3);
    else
        _value |= (1 << 3);
}

Knx::DPT_Status_Mode3::ActiveModeValue Knx::DPT_Status_Mode3::activeMode()
{
    return (ActiveModeValue) (_value & 0x7);
}

void Knx::DPT_Status_Mode3::activeMode(ActiveModeValue value)
{
    _value &= ~ (0x7);
    _value |= (int8_t)value;
}
