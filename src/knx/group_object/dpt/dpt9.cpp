#include "dpt9.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt9::size() const
{
    return Go_2_Octets;
}

void Knx::Dpt9::encode(uint8_t* data) const
{
    float16ToPayload(data, 0, _value, 0xFFFF);
}

bool Knx::Dpt9::decode(uint8_t* data)
{
    if (unsigned16FromPayload(data, 0) == 0x7FFF)
        return false;

    _value = float16FromPayload(data, 0);
    return true;
}

bool Knx::DPT_Value_Temp::decode(uint8_t* data)
{
    Dpt9::decode(data);

    if (_value < -273.0f)
    {
        _value = 0;
        return false;
    }

    return true;
}

void Knx::DPT_Value_Temp::value(float value)
{
    if (value < -273.0f)
        return;

    Dpt9::value(value);
}

bool Knx::DPT_Value_Temp_F::decode(uint8_t* data)
{
    Dpt9::decode(data);

    if (_value < -459.6f)
    {
        _value = 0;
        return false;
    }

    return true;
}

void Knx::DPT_Value_Temp_F::value(float value)
{
    if (value < -459.6f)
        return;

    Dpt9::value(value);
}

bool Knx::Dpt9GeZero::decode(uint8_t* data)
{
    Dpt9::decode(data);

    if (_value < 0)
    {
        _value = 0;
        return false;
    }

    return true;
}

void Knx::Dpt9GeZero::value(float value)
{
    if (value < 0)
        return;

    Dpt9::value(value);
}
