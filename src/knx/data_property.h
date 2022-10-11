#pragma once

#include "property.h"

class DataProperty : public Property
{
  public:
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access);
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access, uint8_t value);
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access, uint16_t value);
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access, uint32_t value);
    DataProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements, uint8_t access, const uint8_t* value);
    ~DataProperty() override;
    uint8_t read(uint16_t start, uint8_t count, uint8_t* data) const override;
    uint8_t write(uint16_t start, uint8_t count, const uint8_t* data) override;
    uint8_t* save(uint8_t* buffer) override;
    const uint8_t* restore(const uint8_t* buffer) override;
    uint16_t saveSize() override;
    const uint8_t* data();
    const uint8_t* data(uint16_t elementIndex);

  private:
    uint16_t _currentElements = 0;
    uint8_t* _data = nullptr;
};
