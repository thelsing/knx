#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt5 : public Dpt
    {
        public:
            Dpt5();
            Dpt5(float value);
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            void value(float value);
            float value() const;
            operator float() const;
            Dpt5& operator=(const float value);

        protected:
            virtual float scale() const
            {
                return 1.0;
            };
            uint8_t _rawValue;
    };

    class DPT_Scaling : public Dpt5
    {
        protected:
            float scale() const override
            {
                return 100.0f;
            }
    };

    class DPT_Angle : public Dpt5
    {
        protected:
            float scale() const override
            {
                return 360.0f;
            }
    };

    class DPT_Value_1_Ucount : protected Dpt5
    {
        public:
            DPT_Value_1_Ucount();
            DPT_Value_1_Ucount(uint8_t value);
            virtual void value(uint8_t value);
            uint8_t value() const;
            operator uint8_t() const;
            DPT_Value_1_Ucount& operator=(const uint8_t value);
    };

    class DPT_Tariff : public DPT_Value_1_Ucount
    {
        public:
            DPT_Tariff();
            DPT_Tariff(uint8_t value);
            bool tariffAvailable();
            void value(uint8_t value) override;
            bool decode(uint8_t* data) override;
    };

    typedef Dpt5 DPT_Percent_U8;
    typedef Dpt5 DPT_DecimalFactor;
} // namespace Knx