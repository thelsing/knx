#pragma once

#include <cstdint>
#include <ctime>

class KNXValue
{
  public:
    KNXValue(bool value);
    KNXValue(uint8_t value);
    KNXValue(uint16_t value);
    KNXValue(uint32_t value);
    KNXValue(uint64_t value);
    KNXValue(int8_t value);
    KNXValue(int16_t value);
    KNXValue(int32_t value);
    KNXValue(int64_t value);
    KNXValue(double value);
    KNXValue(const char* value);
    KNXValue(struct tm value);
    KNXValue(float value);

    operator bool() const;
    operator uint8_t() const;
    operator uint16_t() const;
    operator uint32_t() const;
    operator uint64_t() const;
    operator int8_t() const;
    operator int16_t() const;
    operator int32_t() const;
    operator int64_t() const;
    operator double() const;
    operator const char*() const;
    operator struct tm() const;
    operator float() const;

    KNXValue& operator=(const bool value);
    KNXValue& operator=(const uint8_t value);
    KNXValue& operator=(const uint16_t value);
    KNXValue& operator=(const uint32_t value);
    KNXValue& operator=(const uint64_t value);
    KNXValue& operator=(const int8_t value);
    KNXValue& operator=(const int16_t value);
    KNXValue& operator=(const int32_t value);
    KNXValue& operator=(const int64_t value);
    KNXValue& operator=(const double value);
    KNXValue& operator=(const char* value);
    KNXValue& operator=(const struct tm value);
    KNXValue& operator=(const float value);

  private:
    bool boolValue() const;
    uint8_t ucharValue() const;
    uint16_t ushortValue() const;
    uint32_t uintValue() const;
    uint64_t ulongValue() const;
    int8_t charValue() const;
    int16_t shortValue() const;
    int32_t intValue() const;
    int64_t longValue() const;
    double doubleValue() const;
    const char* stringValue() const;
    struct tm timeValue() const;


    union Value
    {
        bool boolValue;
        uint8_t ucharValue;
        uint16_t ushortValue;
        uint32_t uintValue;
        uint64_t ulongValue;
        int8_t charValue;
        int16_t shortValue;
        int32_t intValue;
        int64_t longValue;
        double doubleValue;
        const char* stringValue;
        struct tm timeValue;
    };
    enum ValueType
    {
        BoolType,
        UCharType,
        UShortType,
        UIntType,
        ULongType,
        CharType,
        ShortType,
        IntType,
        LongType,
        DoubleType,
        StringType,
        TimeType,
    };

    ValueType _type;
    Value _value;
};