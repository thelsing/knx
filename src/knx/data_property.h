#pragma once

#include "property.h"

class DataProperty : public Property
{
  public:
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access);
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access, uint16_t value);
    virtual ~DataProperty() override;
    virtual uint8_t read(uint16_t start, uint8_t count, uint8_t* data) override;
    virtual uint8_t write(uint16_t start, uint8_t count, uint8_t* data) override;
  private:
    uint16_t _currentElements = 0;
    uint8_t* _data = nullptr;
};
