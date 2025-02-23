#include "dpt10.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::DPT_TimeOfDay::size() const
{
    return Go_3_Octets;
}

void Knx::DPT_TimeOfDay::encode(uint8_t* data) const
{
    unsigned8ToPayload(data, 0, (uint8_t)_dow << 5, 0xE0);
    unsigned8ToPayload(data, 0, _hours, 0x1F);
    unsigned8ToPayload(data, 1, _minutes, 0x3F);
    unsigned8ToPayload(data, 2, _seconds, 0x3F);
}

bool Knx::DPT_TimeOfDay::decode(uint8_t* data)
{
    _dow = (DayOfWeekValue)((unsigned8FromPayload(data, 0) & 0xE0) >> 5);
    _hours = unsigned8FromPayload(data, 0) & 0x1F;
    _minutes = unsigned8FromPayload(data, 1) & 0x3F;
    _seconds = unsigned8FromPayload(data, 2) & 0x3F;

    if (_hours > 23 || _minutes > 59 || _seconds > 59)
    {
        _hours = 0;
        _minutes = 0;
        _seconds = 0;
        _dow = NoDay;
        return false;
    }

    return true;
}

Knx::DPT_TimeOfDay::DayOfWeekValue Knx::DPT_TimeOfDay::day() const
{
    return _dow;
}

void Knx::DPT_TimeOfDay::day(DayOfWeekValue value)
{
    _dow = value;
}

uint8_t Knx::DPT_TimeOfDay::hours() const
{
    return _hours;
}

void Knx::DPT_TimeOfDay::hours(uint8_t value)
{
    if (value > 23)
        return;

    _hours = value;
}

uint8_t Knx::DPT_TimeOfDay::minutes() const
{
    return _minutes;
}

void Knx::DPT_TimeOfDay::minutes(uint8_t value)
{
    if (value > 59)
        return;

    _minutes = value;
}

uint8_t Knx::DPT_TimeOfDay::seconds() const
{
    return _seconds;
}

void Knx::DPT_TimeOfDay::seconds(uint8_t value)
{
    if (value > 59)
        return;

    _seconds = value;
}
