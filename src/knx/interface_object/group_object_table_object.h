#pragma once

#include "../group_object/group_object.h"
#include "table_object.h"

namespace Knx
{
    class GroupObjectTableObject : public TableObject
    {
            friend class GroupObject;

        public:
            GroupObjectTableObject(Memory& memory);
            virtual ~GroupObjectTableObject();
            uint16_t entryCount();
            GroupObject& get(uint16_t asap);
            GroupObject& nextUpdatedObject(bool& valid);
            void groupObjects(GroupObject* objs, uint16_t size);

            const uint8_t* restore(const uint8_t* buffer) override;
            const char* name() override
            {
                return "GroupObjectTable";
            }

        protected:
            void beforeStateChange(LoadState& newState) override;

        private:
            void freeGroupObjects();
            bool initGroupObjects();
            uint16_t* _tableData = 0;
            GroupObject* _groupObjects = 0;
            uint16_t _groupObjectCount = 0;
    };
} // namespace Knx