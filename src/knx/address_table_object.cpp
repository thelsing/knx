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
    for (uint16_t i = 1; i <= size; i++)
        if (ntohs(_groupAddresses[i]) == addr)
            return i;
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
    uint16_t size = entryCount();
    for (uint16_t i = 1; i <= size; i++)
        if (ntohs(_groupAddresses[i]) == addr)
            return true;

    return false;
}

void AddressTableObject::beforeStateChange(LoadState& newState)
{
    TableObject::beforeStateChange(newState);
    if (newState != LS_LOADED)
        return;

    _groupAddresses = (uint16_t*)data();
}
