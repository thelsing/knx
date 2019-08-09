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

uint16_t AssociationTableObject::getTSAP(uint16_t idx)
{
    if (idx < 0 || idx >= entryCount())
        return 0;

    return ntohs(_tableData[2 * idx + 1]);
}

uint16_t AssociationTableObject::getASAP(uint16_t idx)
{
    if (idx < 0 || idx >= entryCount())
        return 0;

    return ntohs(_tableData[2 * idx + 2]);
}

uint8_t* AssociationTableObject::save(uint8_t* buffer)
{
    return TableObject::save(buffer);
}

uint8_t* AssociationTableObject::restore(uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);
    _tableData = (uint16_t*)data();
    return buffer;
}

// return type is int32 so that we can return uint16 and -1
int32_t AssociationTableObject::translateAsap(uint16_t asap)
{
    uint16_t entries = entryCount();
    for (uint16_t i = 0; i < entries; i++)
    {
        if (getASAP(i) == asap)
            return getTSAP(i);
    }
    return -1;
}



void AssociationTableObject::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    _tableData = (uint16_t*)data();
}

static PropertyDescription _propertyDescriptions[] =
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_TABLE, false, PDT_GENERIC_04, 65535, ReadLv3 | WriteLv0 },
    { PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3 },
    { PID_TABLE_REFERENCE, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0 },
    { PID_ERROR_CODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 },
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

int32_t AssociationTableObject::nextAsap(uint16_t tsap, uint16_t& startIdx)
{
    uint16_t entries = entryCount();
    for (uint16_t i = startIdx; i < entries; i++)
    {
        startIdx = i+1;

        if (getTSAP(i) == tsap)
        {
            return getASAP(i);
        }
    }
    return -1;
}
