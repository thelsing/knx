#pragma once

#include "property.h"

typedef uint8_t (*PropertyCallback)(uint16_t start, uint8_t count, uint8_t* data);

class CallbackProperty : public Property
{
  public:
    CallbackProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements,
                     uint8_t access, PropertyCallback readCallback, PropertyCallback writeCallback);
    CallbackProperty(PropertyID id, bool writeEnable, PropertyDataType type, uint16_t maxElements,
                     uint8_t access, PropertyCallback readCallback);
    virtual uint8_t read(uint16_t start, uint8_t count, uint8_t* data) override;
    virtual uint8_t write(uint16_t start, uint8_t count, uint8_t* data) override;
  private:
    PropertyCallback _readCallback = nullptr;
    PropertyCallback _writeCallback = nullptr;
};
