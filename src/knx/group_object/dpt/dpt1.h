#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt1: public ValueDpt<bool>
    {
        public:
            Dpt1() {}
            Dpt1(bool value) : ValueDpt(value) {}
            Go_SizeCode size() const override;
            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    template<typename T> class DPT1 : public Dpt1
    {
        public:
            DPT1() {};
            DPT1(T value) : Dpt1(((int)value) == 1) {}
            void value(T value)
            {
                Dpt1::value(((int)value) == 1);
            }
            T value() const
            {
                return Dpt1::value() ? (T)1 : (T)0;
            }
            operator T() const
            {
                return value();
            }
            DPT1& operator=(const T value)
            {
                this->value(value);
                return *this;
            }
    };

    enum SwitchValue
    {
        Off = 0, On = 1
    };
    typedef DPT1<SwitchValue> DPT_Switch;

    typedef Dpt1 DPT_Bool;

    enum EnableValue
    {
        Disable = 0, Enable = 1
    };
    typedef DPT1<EnableValue> DPT_Enable;

    enum RampValue
    {
        NoRamp = 0, Ramp = 1
    };
    typedef DPT1<RampValue> DPT_Ramp;

    enum AlarmValue
    {
        NoAlarm = 0, Alarm = 1
    };
    typedef DPT1<AlarmValue> DPT_Alarm;

    enum BinaryValue
    {
        Low = 0, High = 1
    };
    typedef DPT1<BinaryValue> DPT_BinaryValue;

    enum StepValue
    {
        Decrease = 0, Increase = 1
    };
    typedef DPT1<StepValue> DPT_Step;

    enum UpDownValue
    {
        Up = 0, Down = 1
    };
    typedef DPT1<UpDownValue> DPT_UpDown;

    enum OpenCloseValue
    {
        OpenNormallyOpen = 0, ClosedNormallyOpen = 1
    };
    typedef DPT1<OpenCloseValue> DPT_OpenClose;

    enum StartValue
    {
        Stop = 0, Start = 1
    };
    typedef DPT1<StartValue> DPT_Start;

    enum StateValue
    {
        Inactive = 0, Active = 1
    };
    typedef DPT1<StateValue> DPT_State;

    enum InvertValue
    {
        NotInverted = 0, Inverted = 1
    };
    typedef DPT1<InvertValue> DPT_Invert;

    enum DimSendStyleValue
    {
        StartStop = 0, Cyclically = 1
    };
    typedef DPT1<DimSendStyleValue> DPT_DimSendStyle;

    enum InputSourceValue
    {
        Fixed = 0, Calculated = 1
    };
    typedef DPT1<InputSourceValue> DPT_InputSource;

    enum ResetValue
    {
        NoActionReset = 0, ResetCommand = 1
    };
    typedef DPT1<ResetValue> DPT_Reset;

    enum AckValue
    {
        NoActionAck = 0, AcknowledgeCommand = 1
    };
    typedef DPT1<AckValue> DPT_Ack;

    typedef Dpt1 DPT_Trigger;

    enum OccupancyValue
    {
        NotOccupied = 0, Occupied = 1
    };
    typedef DPT1<OccupancyValue> DPT_Occupancy;

    enum WindowDoorValue
    {
        ClosedNormallyClosed = 0, OpenNormallyClosed = 1
    };
    typedef DPT1<WindowDoorValue> DPT_Window_Door;

    enum LogicalFunctionValue
    {
        OR = 0, AND = 1
    };
    typedef DPT1<LogicalFunctionValue> DPT_LogicalFunction;

    enum SceneABValue
    {
        SceneA = 0, SceneB = 1
    };
    typedef DPT1<SceneABValue> DPT_Scene_AB;

    enum ShutterBlindsMode
    {
        /**
         * Only move up/down
         */
        Shutter = 0,
        /**
         * move up/down and step/stop
         */
        Blinds = 1
    };
    typedef DPT1<ShutterBlindsMode> DPT_ShutterBlinds_Mode;

    enum DayNightValue
    {
        Day = 0, Night = 1
    };
    typedef DPT1<DayNightValue> DPT_DayNight;
}