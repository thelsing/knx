#pragma once

#include "table_object.h"
#include "group_object.h"

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

  protected:
    void beforeStateChange(LoadState& newState) override;

  private:
    void freeGroupObjects();
    bool initGroupObjects();
    uint16_t* _tableData = 0;
    GroupObject* _groupObjects = 0;
    uint16_t _groupObjectCount = 0;
};