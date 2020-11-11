#pragma once

#include "table_object.h"
#include "bits.h"

enum ParameterFloatEncodings
{
    Float_Enc_DPT9 = 0,          // 2 Byte. See Chapter 3.7.2 section 3.10 (Datapoint Types 2-Octet Float Value)
    Float_Enc_IEEE754Single = 1, // 4 Byte. C++ float
    Float_Enc_IEEE754Double = 2, // 8 Byte. C++ double
};
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
