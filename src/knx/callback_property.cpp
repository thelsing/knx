#include "callback_property.h"

CallbackProperty::CallbackProperty(PropertyID id, bool writeEnable, PropertyDataType type,
                                   uint16_t maxElements, uint8_t access,
                                   PropertyCallback readCallback, PropertyCallback writeCallback)
    : Property(id, writeEnable, type, maxElements, access), _readCallback(), _writeCallback()
{}

CallbackProperty::CallbackProperty(PropertyID id, bool writeEnable, PropertyDataType type,
                                   uint16_t maxElements, uint8_t access,
                                   PropertyCallback readCallback)
    : Property(id, writeEnable, type, maxElements, access), _readCallback()
{}

uint8_t CallbackProperty::read(uint16_t start, uint8_t count, uint8_t* data)
{
    if (count == 0 || _readCallback == nullptr)
        return 0;

    return _readCallback(start, count, data);
}

uint8_t CallbackProperty::write(uint16_t start, uint8_t count, uint8_t* data)
{
    if (count == 0 || start > _maxElements || !_writeEnable || start + count > _maxElements + 1
        || _writeCallback == nullptr)
        return 0;
    return _writeCallback(start, count, data);
}
