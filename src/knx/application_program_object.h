#pragma once

#include "table_object.h"

class ApplicationProgramObject : public TableObject
{
  public:
    ApplicationProgramObject(Memory& memory);
    void readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data) override;
    void writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count) override;
    uint8_t propertySize(PropertyID id) override;
    uint8_t* data(uint32_t addr);
    uint8_t getByte(uint32_t addr);
    uint16_t getWord(uint32_t addr);
    uint32_t getInt(uint32_t addr);
    uint8_t* save(uint8_t* buffer) override;
    uint8_t* restore(uint8_t* buffer) override;
    uint16_t saveSize() override;

  protected:
    uint8_t propertyDescriptionCount() override;
    PropertyDescription* propertyDescriptions() override;

  private:
    uint8_t _programVersion[5] = {0, 0, 0, 0, 0};
};