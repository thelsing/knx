#include "property.h"
#include "bits.h"

#include <cstring>

PropertyID Property::Id() const
{
    return _id;
}

bool Property::WriteEnable() const
{
    return _writeEnable;
}

PropertyDataType Property::Type() const
{
    return _type;
}

uint16_t Property::MaxElements() const
{
    return _maxElements;
}

uint8_t Property::Access() const
{
    return _access;
}

uint8_t Property::ElementSize() const
{
    switch (_type)
    {
        case PDT_CHAR:
        case PDT_CONTROL: // is actually 10 if written, but this is always handled with a callback
        case PDT_GENERIC_01:
        case PDT_UNSIGNED_CHAR:
        case PDT_BITSET8:
        case PDT_BINARY_INFORMATION: // only 1 bit really
        case PDT_ENUM8:
        case PDT_SCALING:
            return 1;
        case PDT_GENERIC_02:
        case PDT_INT:
        case PDT_KNX_FLOAT:
        case PDT_UNSIGNED_INT:
        case PDT_VERSION:
        case PDT_BITSET16:
            return 2;
        case PDT_DATE:
        case PDT_ESCAPE:
        case PDT_FUNCTION:
        case PDT_GENERIC_03:
        case PDT_NE_FL:
        case PDT_NE_VL:
        case PDT_POLL_GROUP_SETTING:
        case PDT_TIME:
        case PDT_UTF8:
            return 3;
        case PDT_FLOAT:
        case PDT_GENERIC_04:
        case PDT_LONG:
        case PDT_UNSIGNED_LONG:
            return 4;
        case PDT_GENERIC_05:
        case PDT_SHORT_CHAR_BLOCK:
            return 5;
        case PDT_GENERIC_06:
        case PDT_ALARM_INFO:
            return 6;
        case PDT_GENERIC_07:
            return 7;
        case PDT_DATE_TIME:
        case PDT_DOUBLE:
        case PDT_GENERIC_08:
            return 8;
        case PDT_GENERIC_09:
            return 9;
        case PDT_CHAR_BLOCK:
        case PDT_GENERIC_10:
            return 10;
        case PDT_GENERIC_11:
            return 11;
        case PDT_GENERIC_12:
            return 12;
        case PDT_GENERIC_13:
            return 13;
        case PDT_GENERIC_14:
            return 14;
        case PDT_GENERIC_15:
            return 15;
        case PDT_GENERIC_16:
            return 16;
        case PDT_GENERIC_17:
            return 17;
        case PDT_GENERIC_18:
            return 18;
        case PDT_GENERIC_19:
            return 19;
        case PDT_GENERIC_20:
            return 20;
        default:
            return 0;
    }
}

Property::Property(PropertyID id, bool writeEnable, PropertyDataType type,
                   uint16_t maxElements, uint8_t access)
    : _id(id), _writeEnable(writeEnable), _type(type), _maxElements(maxElements), _access(access)
{}

Property::~Property()
{}


uint8_t Property::read(uint8_t& value) const
{
    if (ElementSize() != 1)
        return 0;
    
    return read(1, 1, &value);
}


uint8_t Property::read(uint16_t& value) const
{
    if (ElementSize() != 2)
        return 0;

    uint8_t data[2];
    uint8_t count = read(1, 1, data);
    if (count > 0)
    {
        popWord(value, data);
    }
    return count;
}


uint8_t Property::read(uint32_t& value) const
{
    if (ElementSize() != 4)
        return 0;

    uint8_t data[4];
    uint8_t count = read(1, 1, data);
    if (count > 0)
    {
        popInt(value, data);
    }
    return count;
}

uint8_t Property::read(uint8_t* value) const
{
    return read(1, 1, value);
}

uint8_t Property::write(uint8_t value)
{
    if (ElementSize() != 1)
        return 0;

    return write(1, 1, &value);
}


uint8_t Property::write(uint16_t value)
{
    if (ElementSize() != 2)
        return 0;

    uint8_t data[2];
    pushWord(value, data);
    return write(1, 1, data);
}


uint8_t Property::write(uint32_t value)
{
    if (ElementSize() != 4)
        return 0;

    uint8_t data[4];
    pushInt(value, data);
    return write(1, 1, data);
}


uint8_t Property::write(const uint8_t* value)
{
    return write(1, 1, value);
}


uint8_t Property::write(uint16_t position, uint16_t value)
{
    if (ElementSize() != 2)
        return 0;

    uint8_t data[2];
    pushWord(value, data);
    return write(position, 1, data);
}

void Property::command(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
{
    (void)data;
    (void)length;
    (void)resultData;
    resultLength = 0;
}

void Property::state(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t &resultLength)
{
    (void)data;
    (void)length;
    (void)resultData;
    resultLength = 0;
}
