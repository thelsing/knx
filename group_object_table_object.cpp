#include <cstring>

#include "group_object_table_object.h"
#include "group_object.h"
#include "bits.h"

GroupObjectTableObject::GroupObjectTableObject(uint8_t* memoryReference): TableObject(memoryReference)
{
    _groupObjects = 0;
    _groupObjectCount = 0;
}

void GroupObjectTableObject::readProperty(PropertyID id, uint32_t start, uint32_t count, uint8_t* data)
{
    switch (id)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_GRP_OBJ_TABLE, data);
            break;
        default:
            TableObject::readProperty(id, start, count, data);
    }
}

uint16_t GroupObjectTableObject::entryCount()
{
    if (loadState() != LS_LOADED)
        return 0;

    return ntohs(_tableData[0]);
}



GroupObject& GroupObjectTableObject::get(uint16_t asap)
{
    return _groupObjects[asap - 1];
}


uint8_t* GroupObjectTableObject::save(uint8_t* buffer)
{
    return TableObject::save(buffer);
}


uint8_t* GroupObjectTableObject::restore(uint8_t* buffer)
{
    buffer = TableObject::restore(buffer);
    
    _tableData = (uint16_t*)_data;
    initGroupObjects();
    
    return buffer;
}

GroupObject& GroupObjectTableObject::nextUpdatedObject(bool& valid)
{
    static uint16_t startIdx = 1;

    uint16_t objCount = entryCount();

    for (uint16_t asap = startIdx; asap <= objCount; asap++)
    {
        GroupObject& go = get(asap);

        if (go.commFlag() == cfUpdate)
        {
            go.commFlag(Ok);
            startIdx = asap + 1;
            valid = true;
            return go;
        }
    }

    startIdx = 1;
    valid = false;
    return get(1);
}

void GroupObjectTableObject::groupObjects(GroupObject * objs, uint16_t size)
{
    _groupObjects = objs;
    _groupObjectCount = size;
    initGroupObjects();
}

void GroupObjectTableObject::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    _tableData = (uint16_t*)_data;

    if (!initGroupObjects())
    {
        newState = LS_ERROR;
        TableObject::_errorCode = E_SOFTWARE_FAULT;
    }
}

bool GroupObjectTableObject::initGroupObjects()
{
    if (!_tableData)
        return false;

    uint16_t goCount = ntohs(_tableData[0]);
    if (goCount != _groupObjectCount)
        return false;

    for (uint16_t asap = 1; asap <= goCount; asap++)
    {
        GroupObject& go = _groupObjects[asap - 1];
        go._asap = asap;
        go._table = this;
        if (go._dataLength != go.goSize())
            return false;
    }
    
    return true;
}
