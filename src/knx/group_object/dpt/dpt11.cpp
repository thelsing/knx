#include "dpt10.h"

#include "dptconvert.h"
#include "dpt11.h"

Knx::Go_SizeCode Knx::DPT_Date::size() const
{
    return Go_3_Octets;
}

void Knx::DPT_Date::encode(uint8_t* data) const
{
    unsigned8ToPayload(data, 0, _day, 0x1F);
    unsigned8ToPayload(data, 1, _month, 0x0F);
    unsigned8ToPayload(data, 2, _year % 100, 0x7F);
}

bool Knx::DPT_Date::decode(uint8_t* data)
{
    _year = unsigned8FromPayload(data, 2) & 0x7F;
    _month = unsigned8FromPayload(data, 1) & 0x0F;
    _day = unsigned8FromPayload(data, 0) & 0x1F;

    if (_year > 99 || _month < 1 || _month > 12 || _day < 1)
    {
        _year = 0;
        _month = 0;
        _day = 0;
        return false;
    }

    _year += _year >= 90 ? 1900 : 2000;
    return true;
}

uint8_t Knx::DPT_Date::day() const
{
    return _day;
}

void Knx::DPT_Date::day(uint8_t value)
{
    if (value < 1 || value > 31)
        return;

    _day = value;
}

uint8_t Knx::DPT_Date::month() const
{
    return _month;
}

void Knx::DPT_Date::month(uint8_t value)
{
    if (value < 1 || value > 12)
        return;

    _month = value;
}

uint16_t Knx::DPT_Date::year() const
{
    return _year;
}

void Knx::DPT_Date::year(uint16_t value)
{
    if (value < 1990 || value > 2089)
        return;

    _year = value;
}
