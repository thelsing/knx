#include <cstring>

#include "group_object_table_object.h"
#include "group_object.h"
#include "bits.h"

GroupObjectTableObject::GroupObjectTableObject(Platform& platform)
    : TableObject(platform)
{
}

GroupObjectTableObject::~GroupObjectTableObject()
{
    freeGroupObjects();
}

void GroupObjectTableObject::readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data)
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

    _tableData = (uint16_t*)data();
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

        if (go.commFlag() == Updated)
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
    freeGroupObjects();
    _groupObjects = objs;
    _groupObjectCount = size;
    initGroupObjects();
}

void GroupObjectTableObject::beforeStateChange(LoadState& newState)
{
    if (newState != LS_LOADED)
        return;

    _tableData = (uint16_t*)data();

    if (!initGroupObjects())
    {
        newState = LS_ERROR;
        TableObject::errorCode(E_SOFTWARE_FAULT);
    }
}

bool GroupObjectTableObject::initGroupObjects()
{
    if (!_tableData)
        return false;
    
    freeGroupObjects();

    uint16_t goCount = ntohs(_tableData[0]);
    
    _groupObjects = new GroupObject[goCount];
    _groupObjectCount = goCount;

    for (uint16_t asap = 1; asap <= goCount; asap++)
    {
        GroupObject& go = _groupObjects[asap - 1];
        go._asap = asap;
        go._table = this;
        
        go._dataLength = go.goSize();
        go._data = new uint8_t[go._dataLength];
        memset(go._data, 0, go._dataLength);
        
        if (go.valueReadOnInit())
            go.requestObjectRead();
    }

    return true;
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3 },
    { PID_TABLE_REFERENCE, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0 },
    { PID_ERROR_CODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 },
};
static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t GroupObjectTableObject::propertyCount()
{
    return _propertyCount;
}


PropertyDescription* GroupObjectTableObject::propertyDescriptions()
{
    return _propertyDescriptions;
}

void GroupObjectTableObject::freeGroupObjects()
{
    if (_groupObjects)
        delete[] _groupObjects;
    
    _groupObjectCount = 0;
    _groupObjects = 0;
}
