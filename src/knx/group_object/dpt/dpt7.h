#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt7: public DPT<uint16_t>
    {
        public:
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

typedef Dpt7 DPT_Value_2_Ucount;
typedef Dpt7 DPT_TimePeriodMsec;
typedef Dpt7 DPT_TimePeriod10MSec;
typedef Dpt7 DPT_TimePeriod100MSec;
typedef Dpt7 DPT_TimePeriodSec;
typedef Dpt7 DPT_TimePeriodMin;
typedef Dpt7 DPT_TimePeriodHrs;
typedef Dpt7 DPT_PropDataType;
typedef Dpt7 DPT_Length_mm;
typedef Dpt7 DPT_UElCurrentmA;
typedef Dpt7 DPT_Brightness;
}