#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt9: public Dpt
    {
        public:
            Dpt9();
            Dpt9(float value);
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            void value(float value);
            float value() const;
            operator float() const;
            Dpt9& operator=(const float value);
        private:
            float _value;
    };

#define DPT_Value_Temp Dpt(9, 1)
#define DPT_Value_Tempd Dpt(9, 2)
#define DPT_Value_Tempa Dpt(9, 3)
#define DPT_Value_Lux Dpt(9, 4)
#define DPT_Value_Wsp Dpt(9, 5)
#define DPT_Value_Pres Dpt(9, 6)
#define DPT_Value_Humidity Dpt(9, 7)
#define DPT_Value_AirQuality Dpt(9, 8)
#define DPT_Value_Time1 Dpt(9, 10)
#define DPT_Value_Time2 Dpt(9, 11)
#define DPT_Value_Volt Dpt(9, 20)
#define DPT_Value_Curr Dpt(9, 21)
#define DPT_PowerDensity Dpt(9, 22)
#define DPT_KelvinPerPercent Dpt(9, 23)
#define DPT_Power Dpt(9, 24)
#define DPT_Value_Volume_Flow Dpt(9, 25)
#define DPT_Rain_Amount Dpt(9, 26)
#define DPT_Value_Temp_F Dpt(9, 27)
#define DPT_Value_Wsp_kmh Dpt(9, 28)
}