#include <cstring>

#include "association_table_object.h"
#include "bits.h"

using namespace std;


AssociationTableObject::AssociationTableObject(Platform& platform)
    : TableObject(platform)
{

}

void AssociationTableObject::readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (id)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_ASSOC_TABLE, data);
            break;
        default:
            TableObject::readProperty(id, start, count, data);
    }
}

uint16_t AssociationTableObject::entryCount()
{
    return ntohs(_tableData[0]);
}

uint16_t AssociationTableObject::operator[](uint16_t idx)
{
    if (idx < 0 || idx >= entryCount())
        return 0;

    return ntohs(_tableData[idx + 1]);
}

uint8_t* AssociationTableObject::save(uint8_t* buffer)
{
    return TableObject::save(buffer);
}

uint8_t* AssociationTableObject::restore(uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);
    _tableData = (uint16_t*)_data;
    return buffer;
}

int32_t AssociationTableObject::translateAsap(uint16_t asap)
{
    uint16_t entries = entryCount();
    for (uint16_t i = 0; i < entries; i++)
    {
        uint16_t entry = operator[](i);
        if (lowByte(entry) == asap)
            return highByte(entry);
    }
    return -1;
}

void AssociationTableObject::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    _tableData = (uint16_t*)_data;
}

static PropertyDescription _propertyDescriptions[] =
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_TABLE, false, PDT_GENERIC_02, 254, ReadLv3 | WriteLv0 }
};
static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t AssociationTableObject::propertyCount()
{
    return _propertyCount;
}


PropertyDescription* AssociationTableObject::propertyDescriptions()
{
    return _propertyDescriptions;
}