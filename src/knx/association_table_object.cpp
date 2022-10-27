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
    #ifdef USE_BINSEARCH
    uint16_t low,high,i;
    low = 0;
    high = entries-1;

    while(low <= high)
    {
        i = (low+high)/2;
        uint16_t asap_i = getASAP(i);
        if (asap_i == asap)
        {
             // as the binary search does not hit the first element in a list with identical items,
             // search downwards to return the first occurence in the table
            while(getASAP(--i) == asap)
                ;
            return getTSAP(i+1);
        }
        if(asap_i > asap)
            high = i - 1;
        else
            low = i + 1 ;
    }
    #else
    for (uint16_t i = 0; i < entries; i++)
    {
        if (getASAP(i) == asap)
            return getTSAP(i);
    }
    #endif
    return -1;
}

void AssociationTableObject::beforeStateChange(LoadState& newState)
{
    TableObject::beforeStateChange(newState);
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
