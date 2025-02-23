#include "dpt5.h"

#include "dptconvert.h"

Knx::Dpt5::Dpt5()
{}

Knx::Dpt5::Dpt5(float value)
{
    this->value(value);
}

Knx::Go_SizeCode Knx::Dpt5::size() const
{
    return Go_1_Octet;
}

void Knx::Dpt5::encode(uint8_t* data) const
{
    unsigned8ToPayload(data, 0, _rawValue, 0xFF);
}

bool Knx::Dpt5::decode(uint8_t* data)
{
    _rawValue = signed8FromPayload(data, 0);
    return true;
}

void Knx::Dpt5::value(float value)
{
    _rawValue = (uint8_t)(value / scale() * 0xFF);
}

float Knx::Dpt5::value() const
{
    return _rawValue / 0xFF * scale();
}

Knx::Dpt5::operator float() const
{
    return value();
}

Knx::Dpt5& Knx::Dpt5::operator=(const float value)
{
    this->value(value);
    return *this;
}

Knx::DPT_Value_1_Ucount::DPT_Value_1_Ucount()
{}

Knx::DPT_Value_1_Ucount::DPT_Value_1_Ucount(uint8_t value)
{
    this->value(value);
}

void Knx::DPT_Value_1_Ucount::value(uint8_t value)
{
    Dpt5::_rawValue = value;
}

uint8_t Knx::DPT_Value_1_Ucount::value() const
{
    return Dpt5::_rawValue;
}

Knx::DPT_Value_1_Ucount::operator uint8_t() const
{
    return value();
}

Knx::DPT_Value_1_Ucount& Knx::DPT_Value_1_Ucount::operator=(const uint8_t value)
{
    this->value(value);
    return *this;
}

Knx::DPT_Tariff::DPT_Tariff()
{}

Knx::DPT_Tariff::DPT_Tariff(uint8_t value)
{
    this->value(value);
}

bool Knx::DPT_Tariff::tariffAvailable()
{
    return _rawValue == 0;
}

void Knx::DPT_Tariff::value(uint8_t value)
{
    if (value == 0xFF)
        return;
}

bool Knx::DPT_Tariff::decode(uint8_t* data)
{
    uint8_t value = signed8FromPayload(data, 0);

    if (value == 0xFF)
        return false;

    _rawValue = value;
    return true;
}
