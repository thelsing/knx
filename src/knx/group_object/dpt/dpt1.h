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

// TODO:
#define DPT_Ramp Dpt1(4)
#define DPT_Alarm Dpt1(5)
#define DPT_BinaryValue Dpt1(6)
#define DPT_Step Dpt1(7)
#define DPT_UpDown Dpt1(8)
#define DPT_OpenClose Dpt1(9)
#define DPT_Start Dpt1(10)
#define DPT_State Dpt1(11)
#define DPT_Invert Dpt1(12)
#define DPT_DimSendStyle Dpt1(13)
#define DPT_InputSource Dpt1(14)
#define DPT_Reset Dpt1(15)
#define DPT_Ack Dpt1(16)
#define DPT_Trigger Dpt1(17)
#define DPT_Occupancy Dpt1(18)
#define DPT_Window_Door Dpt1(19)
#define DPT_LogicalFunction Dpt1(21)
#define DPT_Scene_AB Dpt1(22)
#define DPT_ShutterBlinds_Mode Dpt1(23)
#define DPT_DayNight Dpt1(24)
}