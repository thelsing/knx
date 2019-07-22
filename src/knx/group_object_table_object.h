#pragma once

#include "table_object.h"
#include "group_object.h"

class GroupObjectTableObject : public TableObject
{
    friend class GroupObject;

  public:
    GroupObjectTableObject(Platform& platform);
    virtual ~GroupObjectTableObject();
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    uint16_t entryCount();
    GroupObject& get(uint16_t asap);
    GroupObject& nextUpdatedObject(bool& valid);
    void groupObjects(GroupObject* objs, uint16_t size);

    virtual uint8_t* save(uint8_t* buffer);
    virtual uint8_t* restore(uint8_t* buffer);

  protected:
    virtual void beforeStateChange(LoadState& newState);
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();

  private:
    void freeGroupObjects();
    bool initGroupObjects();
    uint16_t* _tableData = 0;
    GroupObject* _groupObjects = 0;
    uint16_t _groupObjectCount = 0;
};