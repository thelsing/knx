#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt2: public Dpt
    {
        public:
            enum Dpt2Value
            {
                NoControl, Control_Function0, Control_Function1
            };

            Dpt2(unsigned short subgroup = 0);
            Dpt2(Dpt2Value value);
            Go_SizeCode size() const override;

            virtual void encode(uint8_t* data) const override;
            virtual void decode(uint8_t* data) override;

            void value(Dpt2Value value);
            Dpt2Value value() const;
            operator Dpt2Value() const;
            Dpt2& operator=(const Dpt2Value value);
        private:
            Dpt2Value _value;
    };

#define DPT_Switch_Control Dpt2(1)
#define DPT_Bool_Control Dpt2(2)
#define DPT_Enable_Control Dpt2(3)
#define DPT_Ramp_Control Dpt2(4)
#define DPT_Alarm_Control Dpt2(5)
#define DPT_BinaryValue_Control Dpt2(6)
#define DPT_Step_Control Dpt2(7)
#define DPT_Direction1_Control Dpt2(8)
#define DPT_Direction2_Control Dpt2(9)
#define DPT_Start_Control Dpt2(10)
#define DPT_State_Control Dpt2(11)
#define DPT_Invert_Control Dpt2(12)
}