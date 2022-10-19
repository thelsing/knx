#include <cstring>

#include "address_table_object.h"
#include "bits.h"
#include "data_property.h"

using namespace std;

AddressTableObject::AddressTableObject(Memory& memory)
    : TableObject(memory)
{
    Property* properties[] =
    {
        new DataProperty(PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_ADDR_TABLE)
    };

    TableObject::initializeProperties(sizeof(properties), properties);
}

uint16_t AddressTableObject::entryCount()
{
    // after programming without GA the module hangs
    if (loadState() != LS_LOADED || _groupAddresses[0] == 0xFFFF)
        return 0;

    return ntohs(_groupAddresses[0]);
}

uint16_t AddressTableObject::getGroupAddress(uint16_t tsap)
{
    if (loadState() != LS_LOADED || tsap > entryCount() )
        return 0;

    return ntohs(_groupAddresses[tsap]);
}

uint16_t AddressTableObject::getTsap(uint16_t addr)
{
    uint16_t size = entryCount();
    #ifdef USE_BINSEARCH

    uint16_t low,high,i;
    low = 1;
    high = size;

    while(low <= high)
    {
        i = (low+high)/2;
        uint16_t ga = ntohs(_groupAddresses[i]);
        if (ga == addr)
            return i;
        if(addr < ga)
            high = i - 1;
        else
            low = i + 1 ;
    }
    #else
    for (uint16_t i = 1; i <= size; i++)
        if (ntohs(_groupAddresses[i]) == addr)
            return i;
    #endif
    return 0;
}

#pragma region SaveRestore

const uint8_t* AddressTableObject::restore(const uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);

    _groupAddresses = (uint16_t*)data();

    return buffer;
}

#pragma endregion

bool AddressTableObject::contains(uint16_t addr)
{
    return (getTsap(addr) > 0);
}

void AddressTableObject::beforeStateChange(LoadState& newState)
{
    TableObject::beforeStateChange(newState);
    if (newState != LS_LOADED)
        return;

    _groupAddresses = (uint16_t*)data();
}
