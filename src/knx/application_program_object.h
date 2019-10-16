#pragma once

#include "table_object.h"

class ApplicationProgramObject : public TableObject
{
  public:
    ApplicationProgramObject(Platform& platform);
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count);
    uint8_t propertySize(PropertyID id);
    uint8_t* data(uint32_t addr);
    uint8_t getByte(uint32_t addr);
    uint16_t getWord(uint32_t addr);
    uint32_t getInt(uint32_t addr);
    void save();
    void restore(uint8_t* startAddr);
    uint32_t size();

  protected:
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();

  private:
    uint8_t _programVersion[5] = {0, 0, 0, 0, 0};
};
