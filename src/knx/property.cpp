#include "property.h"
#include "bits.h"

#include <cstring>

PropertyID Property::Id()
{
    return _id;
}

bool Property::WriteEnable()
{
    return _writeEnable;
}

PropertyDataType Property::Type()
{
    return _type;
}

uint16_t Property::MaxElements()
{
    return _maxElements;
}

uint8_t Property::Access()
{
    return _access;
}

uint8_t Property::ElementSize()
{
    switch (_type)
    {
        case PDT_CHAR:
        case PDT_CONTROL: // is actually 10 if written, but this is always handled with a callback
        case PDT_GENERIC_01:
        case PDT_UNSIGNED_CHAR:
            return 1;
        case PDT_GENERIC_02:
        case PDT_INT:
        case PDT_KNX_FLOAT:
        case PDT_UNSIGNED_INT:
            return 2;
        case PDT_ALARM_INFO:
        case PDT_BINARY_INFORMATION:
        case PDT_BITSET16:
        case PDT_BITSET8:
        case PDT_DATE:
        case PDT_ENUM8:
        case PDT_ESCAPE:
        case PDT_FUNCTION:
        case PDT_GENERIC_03:
        case PDT_NE_FL:
        case PDT_NE_VL:
        case PDT_POLL_GROUP_SETTING:
        case PDT_SCALING:
        case PDT_TIME:
        case PDT_UTF8:
        case PDT_VERSION:
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
    }
    return 0;
}

Property::Property(PropertyID id, bool writeEnable, PropertyDataType type,
                   uint16_t maxElements, uint8_t access)
    : _id(id), _writeEnable(writeEnable), _type(type), _maxElements(maxElements), _access(access)
{}

Property::~Property()
{}
