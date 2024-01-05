#include "knx_value.h"

#include <cstring>
#include <cstdlib>
#include <ctime>

KNXValue::KNXValue(bool value)
{
    _value.boolValue = value;
    _type = BoolType;
}

KNXValue::KNXValue(uint8_t value)
{
    _value.ucharValue = value;
    _type = UCharType;
}

KNXValue::KNXValue(uint16_t value)
{
    _value.ushortValue = value;
    _type = UShortType;
}

KNXValue::KNXValue(uint32_t value)
{
    _value.uintValue = value;
    _type = UIntType;
}

KNXValue::KNXValue(uint64_t value)
{
    _value.ulongValue = value;
    _type = ULongType;
}

KNXValue::KNXValue(int8_t value)
{
    _value.charValue = value;
    _type = CharType;
}

KNXValue::KNXValue(int16_t value)
{
    _value.shortValue = value;
    _type = ShortType;
}

KNXValue::KNXValue(int32_t value)
{
    _value.intValue = value;
    _type = IntType;
}

KNXValue::KNXValue(int64_t value)
{
    _value.longValue = value;
    _type = LongType;
}

KNXValue::KNXValue(double value)
{
    _value.doubleValue = value;
    _type = DoubleType;
}

KNXValue::KNXValue(const char* value)
{
    _value.stringValue = value;
    _type = StringType;
}

KNXValue::KNXValue(struct tm value)
{
    _value.timeValue = value;
    _type = TimeType;
}

KNXValue::operator bool() const
{
    return boolValue();
}

KNXValue::operator uint8_t() const
{
    return ucharValue();
}

KNXValue::operator uint16_t() const
{
    return ushortValue();
}

KNXValue::operator uint32_t() const
{
    return uintValue();
}

KNXValue::operator uint64_t() const
{
    return ulongValue();
}

KNXValue::operator int8_t() const
{
    return charValue();
}

KNXValue::operator int16_t() const
{
    return shortValue();
}

KNXValue::operator int32_t() const
{
    return intValue();
}

KNXValue::operator int64_t() const
{
    return longValue();
}

KNXValue::operator double() const
{
    return doubleValue();
}

KNXValue::operator const char*() const
{
    return stringValue();
}

KNXValue::operator struct tm() const
{
    return timeValue();
}

KNXValue& KNXValue::operator=(const bool value)
{
    _value.boolValue = value;
    _type = BoolType;
    return *this;
}

KNXValue& KNXValue::operator=(const uint8_t value)
{
    _value.ucharValue = value;
    _type = UCharType;
    return *this;
}

KNXValue& KNXValue::operator=(const uint16_t value)
{
    _value.ushortValue = value;
    _type = UShortType;
    return *this;
}

KNXValue& KNXValue::operator=(const uint32_t value)
{
    _value.uintValue = value;
    _type = UIntType;
    return *this;
}

KNXValue& KNXValue::operator=(const uint64_t value)
{
    _value.ulongValue = value;
    _type = ULongType;
    return *this;
}

KNXValue& KNXValue::operator=(const int8_t value)
{
    _value.charValue = value;
    _type = CharType;
    return *this;
}

KNXValue& KNXValue::operator=(const int16_t value)
{
    _value.shortValue = value;
    _type = ShortType;
    return *this;
}

KNXValue& KNXValue::operator=(const int32_t value)
{
    _value.intValue = value;
    _type = IntType;
    return *this;
}

KNXValue& KNXValue::operator=(const int64_t value)
{
    _value.longValue = value;
    _type = LongType;
    return *this;
}

KNXValue& KNXValue::operator=(const double value)
{
    _value.doubleValue = value;
    _type = DoubleType;
    return *this;
}

KNXValue& KNXValue::operator=(const char* value)
{
    _value.stringValue = value;
    _type = StringType;
    return *this;
}

KNXValue& KNXValue::operator=(const struct tm value)
{
    _value.timeValue = value;
    _type = TimeType;
    return *this;
}

bool KNXValue::boolValue() const
{
    switch (_type)
    {
        case BoolType:
            return _value.boolValue;
        case UCharType:
        case UShortType:
        case UIntType:
        case ULongType:
        case CharType:
        case ShortType:
        case IntType:
        case LongType:
        case TimeType:
            return longValue() != 0;
        case DoubleType:
            return _value.doubleValue != 0;
        case StringType:
            return strcmp(_value.stringValue, "true") == 0 || strcmp(_value.stringValue, "True") == 0 || longValue() != 0 || doubleValue() != 0;
    }
    return 0;
}

uint8_t KNXValue::ucharValue() const
{
    switch (_type)
    {
        case UCharType:
            return _value.ucharValue;
        case BoolType:
        case UShortType:
        case UIntType:
        case ULongType:
        case TimeType:
            return (uint8_t)ulongValue();
        case CharType:
        case ShortType:
        case IntType:
        case LongType:
        case DoubleType:
        case StringType:
            return (uint8_t)longValue();
    }
    return 0;
}

uint16_t KNXValue::ushortValue() const
{
    switch (_type)
    {
        case UShortType:
            return _value.ushortValue;
        case BoolType:
        case UCharType:
        case UIntType:
        case ULongType:
        case TimeType:
            return (uint16_t)ulongValue();
        case CharType:
        case ShortType:
        case IntType:
        case LongType:
        case DoubleType:
        case StringType:
            return (uint16_t)longValue();
    }
    return 0;
}

uint32_t KNXValue::uintValue() const
{
    switch (_type)
    {
        case UIntType:
            return _value.uintValue;
        case BoolType:
        case UCharType:
        case UShortType:
        case ULongType:
        case TimeType:
            return (uint32_t)ulongValue();
        case CharType:
        case ShortType:
        case IntType:
        case LongType:
        case DoubleType:
        case StringType:
            return (uint32_t)longValue();
    }
    return 0;
}

uint64_t KNXValue::ulongValue() const
{
    switch (_type)
    {
        case ULongType:
            return _value.ulongValue;
        case BoolType:
            return _value.boolValue ? 1 : 0;
        case UCharType:
            return (uint64_t)_value.ucharValue;
        case UShortType:
            return (uint64_t)_value.ushortValue;
        case UIntType:
            return (uint64_t)_value.uintValue;
        case TimeType:
        {
            struct tm* timeptr = const_cast<struct tm*>(&_value.timeValue);
            return (uint64_t)mktime(timeptr);
        }
        case CharType:
            return (uint64_t)_value.charValue;
        case ShortType:
            return (uint64_t)_value.shortValue;
        case IntType:
            return (uint64_t)_value.intValue;
        case LongType:
            return (uint64_t)_value.longValue;
        case DoubleType:
            return (uint64_t)_value.doubleValue;
        case StringType:
#ifndef KNX_NO_STRTOx_CONVERSION
            return (uint64_t)strtoul(_value.stringValue, NULL, 0);
#else
            return 0;
#endif
    }
    return 0;
}

int8_t KNXValue::charValue() const
{
    switch (_type)
    {
        case CharType:
            return _value.charValue;
        case BoolType:
        case UCharType:
        case UShortType:
        case UIntType:
        case ULongType:
        case TimeType:
            return (int8_t)ulongValue();
        case ShortType:
        case IntType:
        case LongType:
        case DoubleType:
        case StringType:
            return (int8_t)longValue();
    }
    return 0;
}

int16_t KNXValue::shortValue() const
{
    switch (_type)
    {
        case ShortType:
            return _value.shortValue;
        case BoolType:
        case UCharType:
        case UShortType:
        case UIntType:
        case ULongType:
        case TimeType:
            return (int16_t)ulongValue();
        case CharType:
        case IntType:
        case LongType:
        case DoubleType:
        case StringType:
            return (int16_t)longValue();
    }
    return 0;
}

int32_t KNXValue::intValue() const
{
    switch (_type)
    {
        case IntType:
            return _value.ulongValue;
        case BoolType:
        case UCharType:
        case UShortType:
        case UIntType:
        case ULongType:
        case TimeType:
            return (int32_t)ulongValue();
        case CharType:
        case ShortType:
        case LongType:
        case DoubleType:
        case StringType:
            return (int32_t)longValue();
    }
    return 0;
}

int64_t KNXValue::longValue() const
{
    switch (_type)
    {
        case LongType:
            return _value.longValue;
        case BoolType:
            return _value.boolValue ? 1 : 0;
        case UCharType:
            return (int64_t)_value.ucharValue;
        case UShortType:
            return (int64_t)_value.ushortValue;
        case UIntType:
            return (int64_t)_value.uintValue;
        case ULongType:
            return (int64_t)_value.uintValue;
        case TimeType:
            return (int64_t)ulongValue();
        case CharType:
            return (int64_t)_value.charValue;
        case ShortType:
            return (int64_t)_value.shortValue;
        case IntType:
            return (int64_t)_value.intValue;
        case DoubleType:
            return (int64_t)_value.doubleValue;
        case StringType:
#ifndef KNX_NO_STRTOx_CONVERSION
            return strtol(_value.stringValue, NULL, 0);
#else
            return 0;
#endif
    }
    return 0;
}

double KNXValue::doubleValue() const
{
    switch (_type)
    {
        case DoubleType:
            return _value.doubleValue;
         case BoolType:
            return _value.boolValue ? 1 : 0;
        case UCharType:
            return _value.ucharValue;
        case UShortType:
            return _value.ushortValue;
        case UIntType:
            return _value.uintValue;
        case ULongType:
            return _value.uintValue;
        case TimeType:
            return ulongValue();
        case CharType:
            return _value.charValue;
        case ShortType:
            return _value.shortValue;
        case IntType:
            return _value.intValue;
        case LongType:
            return _value.longValue;
        case StringType:
#ifndef KNX_NO_STRTOx_CONVERSION
            return strtod(_value.stringValue, NULL);
#else
            return 0;
#endif
    }
    return 0;
}

const char* KNXValue::stringValue() const
{
    switch (_type)
    {
        case DoubleType:
        case BoolType:
        case UCharType:
        case UShortType:
        case UIntType:
        case ULongType:
        case TimeType:
        case CharType:
        case ShortType:
        case IntType:
        case LongType:
            return ""; // we would have to manage the memory for the string otherwise. Maybe later.
        case StringType:
            return _value.stringValue;
    }
    return 0;
}

struct tm KNXValue::timeValue() const
{
    switch (_type)
    {
        case TimeType:
            return _value.timeValue;
        case BoolType:
        case UCharType:
        case UShortType:
        case UIntType:
        case ULongType:
        case CharType:
        case ShortType:
        case IntType:
        case LongType:
        case DoubleType:
        case StringType:
        {
            time_t timeVal = ulongValue();
            struct tm timeStruct;
            gmtime_r(&timeVal, &timeStruct);
            return timeStruct;
        }
    }
    struct tm tmp = {0};
    return tmp;
}

KNXValue::KNXValue(float value)
{
    _value.doubleValue = value;
    _type = DoubleType;
}

KNXValue& KNXValue::operator=(const float value)
{
    _value.doubleValue = value;
    _type = DoubleType;
    return *this;
}

KNXValue::operator float() const
{
    return doubleValue();
}