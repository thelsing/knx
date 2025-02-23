#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt12 : public ValueDpt<uint32_t>
    {
        public:
            Dpt12(){};
            Dpt12(uint32_t value)
                : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    typedef Dpt12 DPT_Value_4_Ucount;
    typedef Dpt12 DPT_LongTimePeriod_Sec;
    typedef Dpt12 DPT_LongTimePeriod_Min;
    typedef Dpt12 DPT_LongTimePeriod_Hrs;
} // namespace Knx