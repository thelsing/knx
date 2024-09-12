#include "dpt1.h"

#include "dptconvert.h"

Knx::Dpt1::Dpt1(unsigned short subgroup /*= 0*/) : Dpt(1, subgroup) {}

Knx::Dpt1::Dpt1(bool value) : Dpt1()
{
    _value = value;
}

Knx::Go_SizeCode Knx::Dpt1::size() const
{
    return Go_1_Bit;
}

void Knx::Dpt1::encode(uint8_t* data) const
{
    bitToPayload(data, 7, _value);
}

void Knx::Dpt1::decode(uint8_t* data)
{
    _value = bitFromPayload(data, 7);
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

Knx::DPT_Switch::DPT_Switch() : Dpt1()
{
    subGroup = 1;
}

Knx::DPT_Switch::DPT_Switch(SwitchValue value) : Dpt1(value == On)
{
    subGroup = 1;
}

void Knx::DPT_Switch::value(SwitchValue value)
{
    Dpt1::value(value == On);
}

Knx::DPT_Switch::SwitchValue Knx::DPT_Switch::value() const
{
    return Dpt1::value() ? On : Off;
}

Knx::DPT_Switch::operator Knx::DPT_Switch::SwitchValue() const
{
    return value();
}

Knx::DPT_Switch& Knx::DPT_Switch::operator=(const Knx::DPT_Switch::SwitchValue value)
{
    Dpt1::value(value == On);
    return *this;
}

Knx::DPT_Bool::DPT_Bool() : Dpt1()
{
    subGroup = 2;
}

Knx::DPT_Bool::DPT_Bool(bool value) : DPT_Bool()
{
    Dpt1::value(value);
}

Knx::DPT_Enable::DPT_Enable() : Dpt1()
{
    subGroup = 3;
}

Knx::DPT_Enable::DPT_Enable(EnableValue value) : Dpt1(value == Enable)
{
    subGroup = 3;
}

void Knx::DPT_Enable::value(EnableValue value)
{
    Dpt1::value(value == Enable);
}

Knx::DPT_Enable::EnableValue Knx::DPT_Enable::value() const
{
    return Dpt1::value() ? Enable : Disable;
}

Knx::DPT_Enable::operator Knx::DPT_Enable::EnableValue() const
{
    return value();
}

Knx::DPT_Enable& Knx::DPT_Enable::operator=(const Knx::DPT_Enable::EnableValue value)
{
    Dpt1::value(value == Enable);
    return *this;
}