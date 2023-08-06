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

// after any table change the table is checked if it allows
// binary search access. If not, sortedEntryCount stays 0, 
// otherwise sortedEntryCount represents size of bin search array 
void AssociationTableObject::prepareBinarySearch()
{
    sortedEntryCount = 0;
#ifdef USE_BINSEARCH
    uint16_t lastASAP = 0;
    uint16_t currentASAP = 0;
    uint16_t lookupIdx = 0;
    uint16_t lookupASAP = 0;
    // we iterate through all ASAP
    // the first n ASAP are sorted (strictly increasing number), these are assigning sending TSAP
    // the remaining ASAP have to be all repetitions, otherwise we set sortedEntryCount to 0, which forces linear search 
    if(_tableData != nullptr) {
        for (uint16_t idx = 0; idx < entryCount(); idx++)
        {
            currentASAP = getASAP(idx);
            if (sortedEntryCount)
            {
                // look if the remaining ASAP exist in the previously sorted list.
                while (lookupIdx < sortedEntryCount)
                {
                    lookupASAP = getASAP(lookupIdx);
                    if (currentASAP <= lookupASAP)
                        break; // while
                    else
                        lookupIdx++;
                }
                if (currentASAP < lookupASAP || lookupIdx >= sortedEntryCount)
                {
                    // a new ASAP found, we force linear search
                    sortedEntryCount = 0;
                    break; // for
                }
            }
            else
            {
                // check for strictly increasing ASAP
                if (currentASAP > lastASAP)
                    lastASAP = currentASAP;
                else
                {
                    sortedEntryCount = idx; // last found index indicates end of sorted list
                    idx--; // current item has to be handled as remaining ASAP
                }
            }
        }
        // in case complete table is strictly increasing
        if (lookupIdx == 0 && sortedEntryCount == 0)
            sortedEntryCount = entryCount();
    } 
#endif    
}

const uint8_t* AssociationTableObject::restore(const uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);
    _tableData = (uint16_t*)data();
    prepareBinarySearch();
    return buffer;
}

// return type is int32 so that we can return uint16 and -1
int32_t AssociationTableObject::translateAsap(uint16_t asap)
{
    // sortedEntryCount is determined in prepareBinarySearch()
    // if ETS provides strictly increasing numbers for ASAP
    // represents the size of the array to search
    if (sortedEntryCount)
    {
        uint16_t low = 0;
        uint16_t high = sortedEntryCount - 1;

        while(low <= high)
        {
            uint16_t i = (low + high) / 2;
            uint16_t asap_i = getASAP(i);
            if (asap_i == asap)
                return getTSAP(i);
            if(asap_i > asap)
                high = i - 1;
            else
                low = i + 1 ;
        }
    }
    else
    {
        // if ASAP numbers are not strictly increasing linear seach is used 
        for (uint16_t i = 0; i < entryCount(); i++)
            if (getASAP(i) == asap)
                return getTSAP(i);
    }
    return -1;
}

void AssociationTableObject::beforeStateChange(LoadState& newState)
{
    TableObject::beforeStateChange(newState);
    if (newState != LS_LOADED)
        return;

    _tableData = (uint16_t*)data();
    prepareBinarySearch();
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
