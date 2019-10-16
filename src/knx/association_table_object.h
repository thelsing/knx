#pragma once

#include "table_object.h"

class AssociationTableObject : public TableObject
{
  public:
    AssociationTableObject(Platform& platform);
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);

    void restore(uint8_t* startAddr);

    int32_t translateAsap(uint16_t asap);
    int32_t nextAsap(uint16_t tsap, uint16_t& startIdx);

  protected:
    void beforeStateChange(LoadState& newState);
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();

  private:
    uint16_t entryCount();
    uint16_t getTSAP(uint16_t idx);
    uint16_t getASAP(uint16_t idx);
    uint16_t* _tableData = 0;
};
