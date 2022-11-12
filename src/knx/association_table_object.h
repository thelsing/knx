#pragma once

#include "table_object.h"

class AssociationTableObject : public TableObject
{
  public:
    AssociationTableObject(Memory& memory);

    const uint8_t* restore(const uint8_t* buffer) override;

    int32_t translateAsap(uint16_t asap);
    int32_t nextAsap(uint16_t tsap, uint16_t& startIdx);

  protected:
    void beforeStateChange(LoadState& newState) override;

  private:
    uint16_t entryCount();
    uint16_t getTSAP(uint16_t idx);
    uint16_t getASAP(uint16_t idx);
    void prepareBinarySearch();
    uint16_t* _tableData = 0;
    uint16_t sortedEntryCount;
};
