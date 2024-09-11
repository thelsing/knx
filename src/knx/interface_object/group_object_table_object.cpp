#include "group_object_table_object.h"

#include "../group_object/group_object.h"
#include "../util/logger.h"
#include "../bits.h"

#include <cstring>

#define LOGGER Logger::logger("GroupObjectTableObject")

namespace Knx
{
    GroupObjectTableObject::GroupObjectTableObject(Memory& memory)
        : TableObject(memory)
    {
        Property* properties[]
        {
            new DataProperty(PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_GRP_OBJ_TABLE)
        };
        TableObject::initializeProperties(sizeof(properties), properties);
    }

    GroupObjectTableObject::~GroupObjectTableObject()
    {
        freeGroupObjects();
    }

    uint16_t GroupObjectTableObject::entryCount()
    {
        if (loadState() != LS_LOADED)
            return 0;

        return ntohs(_tableData[0]);
    }

    GroupObject& GroupObjectTableObject::get(uint16_t asap)
    {
        if (asap == 0 || asap > entryCount())
            LOGGER.warning("get: %d is no valid GroupObject. Asap must be > 0 and <= %d", asap, entryCount());

        return _groupObjects[asap - 1];
    }

    const uint8_t* GroupObjectTableObject::restore(const uint8_t* buffer)
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

    void GroupObjectTableObject::groupObjects(GroupObject* objs, uint16_t size)
    {
        freeGroupObjects();
        _groupObjects = objs;
        _groupObjectCount = size;
        initGroupObjects();
    }

    void GroupObjectTableObject::beforeStateChange(LoadState& newState)
    {
        LOGGER.info("beforeStateChange %s", enum_name(newState));
        TableObject::beforeStateChange(newState);

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
        {
            LOGGER.info("initGroupObjects: no table data");
            return false;
        }

        freeGroupObjects();
        LOGGER.info("initGroupObjects %B", _tableData, 40);
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

    void GroupObjectTableObject::freeGroupObjects()
    {
        if (_groupObjects)
            delete[] _groupObjects;

        _groupObjectCount = 0;
        _groupObjects = 0;
    }
}