#include "knx_value.h"

KNXValue::KNXValue()
{}

KNXValue::KNXValue(bool value)
{
    _value.boolValue = value;
}

KNXValue::KNXValue(uint8_t value)
{
    _value.ucharValue = value;
}

KNXValue::KNXValue(uint16_t value)
{
    _value.ushortValue = value;
}

KNXValue::KNXValue(uint32_t value)
{
    _value.uintValue = value;
}

KNXValue::KNXValue(uint64_t value)
{
    _value.ulongValue = value;
}

KNXValue::KNXValue(int8_t value)
{
    _value.charValue = value;
}

KNXValue::KNXValue(int16_t value)
{
    _value.shortValue = value;
}

KNXValue::KNXValue(int32_t value)
{
    _value.intValue = value;
}

KNXValue::KNXValue(int64_t value)
{
    _value.longValue = value;
}

KNXValue::KNXValue(double value)
{
    _value.doubleValue = value;
}

KNXValue::KNXValue(char* value)
{
    _value.stringValue = value;
}

KNXValue::KNXValue(struct tm value)
{
    _value.timeValue = value;
}

KNXValue::operator bool() const
{
    return _value.boolValue;
}

KNXValue::operator uint8_t() const
{
    return _value.ucharValue;
}

KNXValue::operator uint16_t() const
{
    return _value.ushortValue;
}

KNXValue::operator uint32_t() const
{
    return _value.uintValue;
}

KNXValue::operator uint64_t() const
{
    return _value.ulongValue;
}

KNXValue::operator int8_t() const
{
    return _value.charValue;
}

KNXValue::operator int16_t() const
{
    return _value.shortValue;
}

KNXValue::operator int32_t() const
{
    return _value.intValue;
}

KNXValue::operator int64_t() const
{
    return _value.longValue;
}

KNXValue::operator double() const
{
    return _value.doubleValue;
}

KNXValue::operator char*() const
{
    return _value.stringValue;
}

KNXValue::operator struct tm() const
{
    return _value.timeValue;
}

KNXValue& KNXValue::operator=(const bool value)
{
    _value.boolValue = value;
    return *this;
}

bool KNXValue::boolValue() const
{
    return _value.boolValue;
}

uint8_t KNXValue::ucharValue() const
{
    return _value.ucharValue;
}

uint16_t KNXValue::ushortValue() const
{
    return _value.ushortValue;
}

uint32_t KNXValue::uintValue() const
{
    return _value.uintValue;
}

uint64_t KNXValue::ulongValue() const
{
    return _value.ulongValue;
}

int8_t KNXValue::charValue() const
{
    return _value.charValue;
}

int16_t KNXValue::shortValue() const
{
    return _value.shortValue;
}

int32_t KNXValue::intValue() const
{
    return _value.intValue;
}

int64_t KNXValue::longValue() const
{
    return _value.longValue;
}

double KNXValue::doubleValue() const
{
    return _value.doubleValue;
}

char* KNXValue::stringValue() const
{
    return _value.stringValue;
}

struct tm KNXValue::timeValue() const
{
    return _value.timeValue;
}

void KNXValue::boolValue(bool value)
{
    _value.boolValue = value;
}

void KNXValue::ucharValue(uint8_t value)
{
    _value.ucharValue = value;
}

void KNXValue::ushortValue(uint16_t value)
{
    _value.ushortValue = value;
}

void KNXValue::uintValue(uint32_t value)
{
    _value.uintValue = value;
}

void KNXValue::ulongValue(uint64_t value)
{
    _value.ulongValue = value;
}

void KNXValue::charValue(int8_t value)
{
    _value.charValue = value;
}

void KNXValue::shortValue(int16_t value)
{
    _value.shortValue = value;
}

void KNXValue::intValue(int32_t value)
{
    _value.intValue = value;
}

void KNXValue::longValue(int64_t value)
{
    _value.longValue = value;
}

void KNXValue::doubleValue(double value)
{
    _value.doubleValue = value;
}

void KNXValue::stringValue(char* value)
{
    _value.stringValue = value;
}

void KNXValue::timeValue(struct tm value)
{
    _value.timeValue = value;
}
