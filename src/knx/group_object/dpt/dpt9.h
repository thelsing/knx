#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt9 : public ValueDpt<float>
    {
        public:
            Dpt9(){};
            Dpt9(float value)
                : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    class DPT_Value_Temp : public Dpt9
    {
        public:
            DPT_Value_Temp(){};
            DPT_Value_Temp(float value)
                : Dpt9(value) {}
            bool decode(uint8_t* data) override;
            void value(float value) override;
    };

    class DPT_Value_Temp_F : public Dpt9
    {
        public:
            DPT_Value_Temp_F(){};
            DPT_Value_Temp_F(float value)
                : Dpt9(value) {}
            bool decode(uint8_t* data) override;
            void value(float value) override;
    };

    class Dpt9GeZero : public Dpt9
    {
        public:
            Dpt9GeZero(){};
            Dpt9GeZero(float value)
                : Dpt9(value) {}
            bool decode(uint8_t* data) override;
            void value(float value) override;
    };

    typedef Dpt9 DPT_Value_Tempd;
    typedef Dpt9 DPT_Value_Tempa;
    typedef Dpt9GeZero DPT_Value_Lux;
    typedef Dpt9GeZero DPT_Value_Wsp;
    typedef Dpt9GeZero DPT_Value_Pres;
    typedef Dpt9GeZero DPT_Value_Humidity;
    typedef Dpt9GeZero DPT_Value_AirQuality;
    typedef Dpt9 DPT_Value_Time1;
    typedef Dpt9 DPT_Value_Time2;
    typedef Dpt9 DPT_Value_Volt;
    typedef Dpt9 DPT_Value_Curr;
    typedef Dpt9 DPT_PowerDensity;
    typedef Dpt9 DPT_KelvinPerPercent;
    typedef Dpt9 DPT_Power;
    typedef Dpt9 DPT_Value_Volume_Flow;
    typedef Dpt9 DPT_Rain_Amount;
    typedef Dpt9 DPT_Value_Wsp_kmh;
    typedef Dpt9GeZero DPT_Value_Absolute_Humidity;
    typedef Dpt9GeZero DPT_Concentration_ugm3;
} // namespace Knx