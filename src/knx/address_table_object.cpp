#include <cstring>

#include "address_table_object.h"
#include "bits.h"

using namespace std;


AddressTableObject::AddressTableObject(Platform& platform)
    : TableObject(platform)
{

}

void AddressTableObject::readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (id)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_ADDR_TABLE, data);
            break;
        default:
            TableObject::readProperty(id, start, count, data);
    }
}

uint16_t AddressTableObject::entryCount()
{
    if (loadState() != LS_LOADED)
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


void AddressTableObject::restore(uint8_t* startAddr)
{
    TableObject::restore(startAddr);

    _groupAddresses = (uint16_t*)data();
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
    if (newState != LS_LOADED)
        return;

    _groupAddresses = (uint16_t*)data();
}

static PropertyDescription _propertyDescriptions[] =
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3 },
    { PID_TABLE_REFERENCE, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0 },
    { PID_ERROR_CODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 },
};

static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t AddressTableObject::propertyCount()
{
    return _propertyCount;
}


PropertyDescription* AddressTableObject::propertyDescriptions()
{
    return _propertyDescriptions;
}