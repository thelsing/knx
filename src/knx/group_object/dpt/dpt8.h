#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt8: public ValueDpt<int16_t>
    {
        public:
            Dpt8() {};
            Dpt8(int16_t value) : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    typedef Dpt8 DPT_Value_2_Count;
    typedef Dpt8 DPT_DeltaTimeMsec;
    typedef Dpt8 DPT_DeltaTime10MSec;
    typedef Dpt8 DPT_DeltaTime100MSec;
    typedef Dpt8 DPT_DeltaTimeSec;
    typedef Dpt8 DPT_DeltaTimeMin;
    typedef Dpt8 DPT_DeltaTimeHrs;
    typedef Dpt8 DPT_Percent_V16;
    typedef Dpt8 DPT_Rotation_Angle;
}