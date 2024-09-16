#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt13: public ValueDpt<int32_t>
    {
        public:
            Dpt13() {};
            Dpt13(int32_t value) : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    typedef Dpt13 DPT_Value_4_Count;
    typedef Dpt13 DPT_FlowRate_m3_h;
    typedef Dpt13 DPT_ActiveEnergy;
    typedef Dpt13 DPT_ApparantEnergy;
    typedef Dpt13 DPT_ReactiveEnergy;
    typedef Dpt13 DPT_ActiveEnergy_kWh;
    typedef Dpt13 DPT_ApparantEnergy_kVAh;
    typedef Dpt13 DPT_ReactiveEnergy_kVARh;
    typedef Dpt13 DPT_ActiveEnergy_MWh;
    typedef Dpt13 DPT_LongDeltaTimeSec;
}