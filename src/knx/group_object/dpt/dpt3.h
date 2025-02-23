#pragma once
#include "dpt.h"
#include "dpt1.h"
#include "dptconvert.h"
namespace Knx
{
    template <typename T>
    class DPT3 : public Dpt
    {
        public:
            Go_SizeCode size() const override
            {
                return Go_4_Bit;
            }

            void encode(uint8_t* data) const override
            {
                bitToPayload(data, 4, ((int)_direction) == 1);
                unsigned8ToPayload(data, 0, _stepCode, 0x07);
            }

            bool decode(uint8_t* data) override
            {
                bool c = bitFromPayload(data, 4);
                _direction = c ? (T)1 : (T)0;
                _stepCode = unsigned8FromPayload(data, 0) & 0x07;
                return true;
            }

            T direction() const
            {
                return _direction;
            }

            void direction(T value)
            {
                _direction = value;
            }

            float stepPercent() const
            {
                if (_stepCode == 0)
                    return 0.0f;

                return 100.0f / (0x1 << (_stepCode - 1));
            }

            uint8_t stepCode() const
            {
                return _stepCode;
            }

            void stepCode(uint8_t value)
            {
                _stepCode = value & 0x7;
            }

            bool stop() const
            {
                return _stepCode == 0;
            }

        private:
            T _direction;
            uint8_t _stepCode;
    };

    typedef DPT3<StepValue> DPT_Control_Dimming;
    typedef DPT3<UpDownValue> DPT_Control_Blinds;
} // namespace Knx