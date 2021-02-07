#pragma once

#include "table_object.h"
#include "bits.h"

class ApplicationProgramObject : public TableObject
{
  public:
    ApplicationProgramObject(Memory& memory);
    uint8_t* data(uint32_t addr);
    uint8_t getByte(uint32_t addr);
    uint16_t getWord(uint32_t addr);
    uint32_t getInt(uint32_t addr);
    double getFloat(uint32_t addr, ParameterFloatEncodings encoding);
};
