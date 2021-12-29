#include <cstring>

#include "association_table_object.h"
#include "bits.h"
#include "data_property.h"

using namespace std;

AssociationTableObject::AssociationTableObject(Memory& memory)
    : TableObject(memory)
{
    Property* properties[] =
    {
        new DataProperty(PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_ASSOC_TABLE),
        new DataProperty(PID_TABLE, false, PDT_GENERIC_04, 65535, ReadLv3 | WriteLv0) //FIXME: implement correctly
    };

    TableObject::initializeProperties(sizeof(properties), properties);
}

uint16_t AssociationTableObject::entryCount()
{
    return ntohs(_tableData[0]);
}

uint16_t AssociationTableObject::getTSAP(uint16_t idx)
{
    if (idx >= entryCount())
        return 0;

    return ntohs(_tableData[2 * idx + 1]);
}

uint16_t AssociationTableObject::getASAP(uint16_t idx)
{
    if (idx >= entryCount())
        return 0;

    return ntohs(_tableData[2 * idx + 2]);
}

const uint8_t* AssociationTableObject::restore(const uint8_t* buffer)
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
