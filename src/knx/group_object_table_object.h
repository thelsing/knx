#pragma once

#include "table_object.h"
#include "group_object.h"
#include "platform.h"

class GroupObjectTableObject : public TableObject
{
        friend class GroupObject;

    public:
        GroupObjectTableObject(Memory& memory, Platform& platform);
        virtual ~GroupObjectTableObject();
        uint16_t entryCount();
        GroupObject& get(uint16_t asap);
        GroupObject& nextUpdatedObject(bool& valid);
        void groupObjects(GroupObject* objs, uint16_t size);

        const uint8_t* restore(const uint8_t* buffer) override;

    protected:
        void beforeStateChange(LoadState& newState) override;

    private:
        void freeGroupObjects();
        bool initGroupObjects();
        uint16_t* _tableData = 0;
        Platform& _platform;
        GroupObject* _groupObjects = 0;
        uint16_t _groupObjectCount = 0;
};