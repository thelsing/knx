#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt1: public Dpt
    {
        public:
            Dpt1(unsigned short subgroup = 0);
            Dpt1(bool value);
            Go_SizeCode size() const override;

            virtual void encode(uint8_t* data) const override;
            virtual void decode(uint8_t* data) override;

            void value(bool value);
            bool value() const;
            operator bool() const;
            Dpt1& operator=(const bool value);
        private:
            bool _value;
    };

    class DPT_Switch : public Dpt1
    {
        public:
            enum SwitchValue
            {
                Off, On
            };

            DPT_Switch();
            DPT_Switch(SwitchValue value);
            void value(SwitchValue value);
            SwitchValue value() const;
            operator SwitchValue() const;
            DPT_Switch& operator=(const SwitchValue value);
    };

    class DPT_Bool : public Dpt1
    {
        public:
            DPT_Bool();
            DPT_Bool(bool value);
    };

    class DPT_Enable : public Dpt1
    {
        public:
            enum EnableValue
            {
                Disable, Enable
            };

            DPT_Enable();
            DPT_Enable(EnableValue value);
            void value(EnableValue value);
            EnableValue value() const;
            operator EnableValue() const;
            DPT_Enable& operator=(const EnableValue value);
    };
}