#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt6 : public ValueDpt<int8_t>
    {
        public:
            Dpt6(){};
            Dpt6(int8_t value)
                : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    typedef Dpt6 DPT_Percent_V8;
    typedef Dpt6 DPT_Value_1_Count;

    class DPT_Status_Mode3 : public Dpt6
    {
        public:
            bool decode(uint8_t* data) override;
            enum SetClearValue
            {
                Set = 0,
                Clear = 1
            };
            enum ActiveModeValue
            {
                Mode0Active = 0x1,
                Mode1Active = 0x2,
                Mode2Active = 0x4
            };

            SetClearValue A();
            void A(SetClearValue value);

            SetClearValue B();
            void B(SetClearValue value);

            SetClearValue C();
            void C(SetClearValue value);

            SetClearValue D();
            void D(SetClearValue value);

            SetClearValue E();
            void E(SetClearValue value);

            ActiveModeValue activeMode();
            void activeMode(ActiveModeValue value);
    };
} // namespace Knx