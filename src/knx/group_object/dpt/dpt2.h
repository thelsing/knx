#pragma once
#include "dpt.h"
#include "dpt1.h"
#include "dptconvert.h"

namespace Knx
{
    enum ControlValue
    {
        NoControl,
        Control
    };
    template <typename T>
    class DPT2 : public ValueDpt<T>
    {
        public:
            DPT2(){};
            DPT2(bool value)
                : ValueDpt<T>(value) {}
            Go_SizeCode size() const override
            {
                return Go_2_Bit;
            }

            void encode(uint8_t* data) const override
            {
                if (_control == NoControl)
                {
                    bitToPayload(data, 6, false);
                    return;
                }

                bitToPayload(data, 6, true);
                bitToPayload(data, 7, ((int)ValueDpt<T>::value()) == 1);
            }

            bool decode(uint8_t* data) override
            {
                bool c = bitFromPayload(data, 6);

                if (!c)
                {
                    _control = NoControl;
                    return true;
                }

                bool v = bitFromPayload(data, 7);

                value(v ? (T)1 : (T)0);
                return true;
            }

            void control(ControlValue control)
            {
                _control = control;
            }

            ControlValue control() const
            {
                return _control;
            }

        private:
            ControlValue _control;
            T _value;
    };

    typedef DPT2<SwitchValue> DPT_Switch_Control;
    typedef DPT2<bool> DPT_Bool_Control;
    typedef DPT2<EnableValue> DPT_Enable_Control;
    typedef DPT2<RampValue> DPT_Ramp_Control;
    typedef DPT2<AlarmValue> DPT_Alarm_Control;
    typedef DPT2<BinaryValue> DPT_BinaryValue_Control;
    typedef DPT2<StepValue> DPT_Step_Control;
    typedef DPT2<UpDownValue> DPT_Direction1_Control;
    typedef DPT2<UpDownValue> DPT_Direction2_Control;
    typedef DPT2<StartValue> DPT_Start_Control;
    typedef DPT2<StateValue> DPT_State_Control;
    typedef DPT2<InvertValue> DPT_Invert_Control;
} // namespace Knx