#include "dpt15.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::Dpt15::size() const
{
    return Go_4_Octets;
}

void Knx::Dpt15::encode(uint8_t* data) const
{

    for (int n = 0, factor = 100000; n < 6; ++n, factor /= 10)
        bcdToPayload(data, n, (_accessCode / factor) % 10);

    bitToPayload(data, 24, _detectionError);
    bitToPayload(data, 25, _permission);
    bitToPayload(data, 26, _readDirection == RightToLeft);
    bitToPayload(data, 27, _encrypted);
    bcdToPayload(data, 7, _index);
}

bool Knx::Dpt15::decode(uint8_t* data)
{
    int32_t digits = 0;

    for (int n = 0, factor = 100000; n < 6; ++n, factor /= 10)
    {
        unsigned char digit = bcdFromPayload(data, n);

        if (digit > 9)
            return false;

        digits += digit * factor;
    }

    _accessCode = digits;
    _detectionError = bitFromPayload(data, 24);
    _permission = bitFromPayload(data, 25);
    _readDirection = bitFromPayload(data, 26) ? RightToLeft : LeftToRight;
    _encrypted = bitFromPayload(data, 27);
    _index = bcdFromPayload(data, 7);
    return true;
}

uint32_t Knx::Dpt15::accessCode() const
{
    return _accessCode;
}

void Knx::Dpt15::accessCode(const uint32_t value)
{
    if (value > 999999)
        return;

    _accessCode = value;
}

bool Knx::Dpt15::detectionError() const
{
    return _detectionError;
}

void Knx::Dpt15::detetionError(const bool value)
{
    _detectionError = value;
}

bool Knx::Dpt15::permission() const
{
    return _permission;
}

void Knx::Dpt15::permission(const bool value)
{
    _permission = value;
}

Knx::Dpt15::ReadDirectionValue Knx::Dpt15::readDirection() const
{
    return _readDirection;
}

void Knx::Dpt15::readDirection(const ReadDirectionValue value)
{
    _readDirection = value;
}

bool Knx::Dpt15::encrypted() const
{
    return _encrypted;
}

void Knx::Dpt15::encrypted(const bool value)
{
    _encrypted = value;
}

uint8_t Knx::Dpt15::index() const
{
    return _index;
}

void Knx::Dpt15::index(const uint8_t value)
{
    if (value > 15)
        return;

    _index = value;
}
