#pragma once

#include <cstdint>
#include <ctime>

class KNXValue
{
  public:
    KNXValue();
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
    KNXValue(char* value);
    KNXValue(struct tm value);

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
    operator char*() const;
    operator struct tm() const;

    KNXValue& operator=(const bool value);

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
    char* stringValue() const;
    struct tm timeValue() const;

    void boolValue(bool value);
    void ucharValue(uint8_t value);
    void ushortValue(uint16_t value);
    void uintValue(uint32_t value);
    void ulongValue(uint64_t value);
    void charValue(int8_t value);
    void shortValue(int16_t value);
    void intValue(int32_t value);
    void longValue(int64_t value);
    void doubleValue(double value);
    void stringValue(char* value);
    void timeValue(struct tm value);

  private:
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
        char* stringValue;
        struct tm timeValue;
    };
    Value _value;
};