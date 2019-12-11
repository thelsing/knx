#pragma once

#include "table_object.h"

class AssociationTableObject : public TableObject
{
  public:
    AssociationTableObject(Memory& memory);
    void readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data) override;
    ObjectType objectType() override { return OT_ASSOC_TABLE; }

    uint8_t* restore(uint8_t* buffer) override;

    int32_t translateAsap(uint16_t asap);
    int32_t nextAsap(uint16_t tsap, uint16_t& startIdx);

  protected:
    void beforeStateChange(LoadState& newState) override;
    uint8_t propertyDescriptionCount() override;
    PropertyDescription* propertyDescriptions() override;

  private:
    uint16_t entryCount();
    uint16_t getTSAP(uint16_t idx);
    uint16_t getASAP(uint16_t idx);
    uint16_t* _tableData = 0;
};
