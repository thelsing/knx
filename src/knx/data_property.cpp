#include "data_property.h"
#include "bits.h"

#include <cstring>

uint8_t DataProperty::read(uint16_t start, uint8_t count, uint8_t* data)
{
    if (count == 0 || _currentElements == 0 || start > _currentElements || count > _currentElements - start + 1)
        return 0;

    if (start == 0)
    {
        pushWord(_currentElements, data);
        return 1;
    }

    // we start counting with zero
    start -= 1;

    // data is already big enough to hold the data
    memcpy(data, _data + start, count * ElementSize());

    return count;
}

uint8_t DataProperty::write(uint16_t start, uint8_t count, uint8_t* data)
{
    if (count == 0 || start > _maxElements || !_writeEnable || start + count > _maxElements + 1)
        return 0;

    if (start == 0)
    {
        if (count == 1 && data[0] == 0 && data[1] == 0)
        {
            // reset _data
            _currentElements = 0;
            if (_data)
            {
                delete[] _data;
                _data = nullptr;
            }
            return 1;
        }
        else
            return 0;
    }

    // we start counting with zero
    start -= 1;
    if (start + count > _currentElements)
    {
        //reallocate memory for _data
        uint8_t* oldData = _data;
        size_t oldDataSize = _currentElements * ElementSize();

        size_t newDataSize = (start + count) * ElementSize();
        _data = new uint8_t[newDataSize];
        memset(_data, 0, newDataSize);

        if (oldData != nullptr)
        {
            memcpy(_data, oldData, oldDataSize);
            delete[] oldData;
        }

        _currentElements = start + count;
    }

    memcpy(_data + start, data, count * ElementSize());

    return count;
}

DataProperty::DataProperty(PropertyID id, bool writeEnable, PropertyDataType type,
                           uint16_t maxElements, uint8_t access)
    : Property(id, writeEnable, type, maxElements, access)
{}

DataProperty::~DataProperty()
{
    if (_data)
        delete[] _data;
}

DataProperty::DataProperty(PropertyID id, bool writeEnable, PropertyDataType type,
                           uint16_t maxElements, uint8_t access, uint16_t value)
    : Property(id, writeEnable, type, maxElements, access)
{
    uint8_t elementSize = ElementSize();
    if (elementSize == 2)
    {
        uint8_t data[elementSize];
        pushWord(value, data);
        write(1, 1, data);
    }
}
