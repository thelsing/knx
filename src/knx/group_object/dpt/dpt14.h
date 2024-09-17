#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt14: public ValueDpt<float>
    {
        public:
            Dpt14() {};
            Dpt14(float value) : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };
    
    typedef Dpt14 DPT_Value_Acceleration;
    typedef Dpt14 DPT_Value_Acceleration_Angular;
    typedef Dpt14 DPT_Value_Activation_Energy;
    typedef Dpt14 DPT_Value_Activity;
    typedef Dpt14 DPT_Value_Mol;
    typedef Dpt14 DPT_Value_Amplitude;
    typedef Dpt14 DPT_Value_AngleRad;
    typedef Dpt14 DPT_Value_AngleDeg;
    typedef Dpt14 DPT_Value_Angular_Momentum;
    typedef Dpt14 DPT_Value_Angular_Velocity;
    typedef Dpt14 DPT_Value_Area;
    typedef Dpt14 DPT_Value_Capacitance;
    typedef Dpt14 DPT_Value_Charge_DensitySurface;
    typedef Dpt14 DPT_Value_Charge_DensityVolume;
    typedef Dpt14 DPT_Value_Compressibility;
    typedef Dpt14 DPT_Value_Conductance;
    typedef Dpt14 DPT_Value_Electrical_Conductivity;
    typedef Dpt14 DPT_Value_Density;
    typedef Dpt14 DPT_Value_Electric_Charge;
    typedef Dpt14 DPT_Value_Electric_Current;
    typedef Dpt14 DPT_Value_Electric_CurrentDensity;
    typedef Dpt14 DPT_Value_Electric_DipoleMoment;
    typedef Dpt14 DPT_Value_Electric_Displacement;
    typedef Dpt14 DPT_Value_Electric_FieldStrength;
    typedef Dpt14 DPT_Value_Electric_Flux;
    typedef Dpt14 DPT_Value_Electric_FluxDensity;
    typedef Dpt14 DPT_Value_Electric_Polarization;
    typedef Dpt14 DPT_Value_Electric_Potential;
    typedef Dpt14 DPT_Value_Electric_PotentialDifference;
    typedef Dpt14 DPT_Value_ElectromagneticMoment;
    typedef Dpt14 DPT_Value_Electromotive_Force;
    typedef Dpt14 DPT_Value_Energy;
    typedef Dpt14 DPT_Value_Force;
    typedef Dpt14 DPT_Value_Frequency;
    typedef Dpt14 DPT_Value_Angular_Frequency;
    typedef Dpt14 DPT_Value_Heat_Capacity;
    typedef Dpt14 DPT_Value_Heat_FlowRate;
    typedef Dpt14 DPT_Value_Heat_Quantity;
    typedef Dpt14 DPT_Value_Impedance;
    typedef Dpt14 DPT_Value_Length;
    typedef Dpt14 DPT_Value_Light_Quantity;
    typedef Dpt14 DPT_Value_Luminance;
    typedef Dpt14 DPT_Value_Luminous_Flux;
    typedef Dpt14 DPT_Value_Luminous_Intensity;
    typedef Dpt14 DPT_Value_Magnetic_FieldStrength;
    typedef Dpt14 DPT_Value_Magnetic_Flux;
    typedef Dpt14 DPT_Value_Magnetic_FluxDensity;
    typedef Dpt14 DPT_Value_Magnetic_Moment;
    typedef Dpt14 DPT_Value_Magnetic_Polarization;
    typedef Dpt14 DPT_Value_Magnetization;
    typedef Dpt14 DPT_Value_MagnetomotiveForce;
    typedef Dpt14 DPT_Value_Mass;
    typedef Dpt14 DPT_Value_MassFlux;
    typedef Dpt14 DPT_Value_Momentum;
    typedef Dpt14 DPT_Value_Phase_AngleRad;
    typedef Dpt14 DPT_Value_Phase_AngleDeg;
    typedef Dpt14 DPT_Value_Power;
    typedef Dpt14 DPT_Value_Power_Factor;
    typedef Dpt14 DPT_Value_Pressure;
    typedef Dpt14 DPT_Value_Reactance;
    typedef Dpt14 DPT_Value_Resistance;
    typedef Dpt14 DPT_Value_Resistivity;
    typedef Dpt14 DPT_Value_SelfInductance;
    typedef Dpt14 DPT_Value_SolidAngle;
    typedef Dpt14 DPT_Value_Sound_Intensity;
    typedef Dpt14 DPT_Value_Speed;
    typedef Dpt14 DPT_Value_Stress;
    typedef Dpt14 DPT_Value_Surface_Tension;
    typedef Dpt14 DPT_Value_Common_Temperature;
    typedef Dpt14 DPT_Value_Absolute_Temperature;
    typedef Dpt14 DPT_Value_TemperatureDifference;
    typedef Dpt14 DPT_Value_Thermal_Capacity;
    typedef Dpt14 DPT_Value_Thermal_Conductivity;
    typedef Dpt14 DPT_Value_ThermoelectricPower;
    typedef Dpt14 DPT_Value_Time;
    typedef Dpt14 DPT_Value_Torque;
    typedef Dpt14 DPT_Value_Volume;
    typedef Dpt14 DPT_Value_Volume_Flux;
    typedef Dpt14 DPT_Value_Weight;
    typedef Dpt14 DPT_Value_Work;
    typedef Dpt14 DPT_Value_ApparentPower;
}