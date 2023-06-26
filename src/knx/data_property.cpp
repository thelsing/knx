#include "data_property.h"
#include "bits.h"

#include <cstring>

uint8_t DataProperty::read(uint16_t start, uint8_t count, uint8_t* data) const
{
    if (start == 0)
    {
        pushWord(_currentElements, data);
        return 1;
    }

    if (count == 0 || _currentElements == 0 || start > _currentElements || count > _currentElements - start + 1)
        return 0;


    // we start counting with zero
    start -= 1;

    // data is already big enough to hold the data
    memcpy(data, _data + (start * ElementSize()), count * ElementSize()); 

    return count;
}

uint8_t DataProperty::write(uint16_t start, uint8_t count, const uint8_t* data)
{
    if (count == 0 || start > _maxElements || start + count > _maxElements + 1)
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
        // reallocate memory for _data
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

    memcpy(_data + (start * ElementSize()), data, count * ElementSize());

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
    Property::write(value);
}

DataProperty::DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, 
                           uint16_t maxElements, uint8_t access, uint32_t value)
    : Property(id, writeEnable, type, maxElements, access)
{
    Property::write(value);
}

DataProperty::DataProperty(PropertyID id, bool writeEnable, PropertyDataType type,
                           uint16_t maxElements, uint8_t access, uint8_t value)
    : Property(id, writeEnable, type, maxElements, access)
{
    Property::write(value);
}

DataProperty::DataProperty(PropertyID id, bool writeEnable, PropertyDataType type,
                           uint16_t maxElements, uint8_t access, const uint8_t* value)
    : Property(id, writeEnable, type, maxElements, access)
{
    Property::write(value);
}

uint16_t DataProperty::saveSize()
{
    return sizeof(_currentElements) + _maxElements * ElementSize();
}


const uint8_t* DataProperty::restore(const uint8_t* buffer)
{
    uint16_t elements = 0;
    buffer = popWord(elements, buffer);

    if (elements != _currentElements)
    {
        if (_data != nullptr)
            delete[] _data;
        
        _data = new uint8_t[elements * ElementSize()];
        _currentElements = elements;
    }

    if (elements > 0)
        buffer = popByteArray(_data, elements * ElementSize(), buffer);

    return buffer;
}


uint8_t* DataProperty::save(uint8_t* buffer)
{
    buffer = pushWord(_currentElements, buffer);
    if (_currentElements > 0)
        buffer = pushByteArray(_data, _currentElements * ElementSize(), buffer);

    return buffer;
}


const uint8_t* DataProperty::data()
{
    return _data;
}

const uint8_t* DataProperty::data(uint16_t elementIndex)
{
    if ((elementIndex == 0) || (elementIndex > _currentElements))
        return nullptr;

    elementIndex -= 1; // Starting from 0
    uint16_t offset = elementIndex * ElementSize();
    return _data + offset;
}
