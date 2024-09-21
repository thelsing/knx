#pragma once

#include "../group_object.h"
namespace Knx
{
#define DPT_String_ASCII Dpt(16, 0)
#define DPT_String_8859_1 Dpt(16, 1)
#define DPT_SceneNumber Dpt(17, 1)
#define DPT_SceneControl Dpt(18, 1)
#define DPT_DateTime Dpt(19, 1)
#define DPT_SCLOMode Dpt(20, 1)
#define DPT_BuildingMode Dpt(20, 2)
#define DPT_OccMode Dpt(20, 3)
#define DPT_Priority Dpt(20, 4)
#define DPT_LightApplicationMode Dpt(20, 5)
#define DPT_ApplicationArea Dpt(20, 6)
#define DPT_AlarmClassType Dpt(20, 7)
#define DPT_PSUMode Dpt(20, 8)
#define DPT_ErrorClass_System Dpt(20, 11)
#define DPT_ErrorClass_HVAC Dpt(20, 12)
#define DPT_Time_Delay Dpt(20, 13)
#define DPT_Beaufort_Wind_Force_Scale Dpt(20, 14)
#define DPT_SensorSelect Dpt(20, 17)
#define DPT_ActuatorConnectType Dpt(20, 20)
#define DPT_FuelType Dpt(20, 100)
#define DPT_BurnerType Dpt(20, 101)
#define DPT_HVACMode Dpt(20, 102)
#define DPT_DHWMode Dpt(20, 103)
#define DPT_LoadPriority Dpt(20, 104)
#define DPT_HVACContrMode Dpt(20, 105)
#define DPT_HVACEmergMode Dpt(20, 106)
#define DPT_ChangeoverMode Dpt(20, 107)
#define DPT_ValveMode Dpt(20, 108)
#define DPT_DamperMode Dpt(20, 109)
#define DPT_HeaterMode Dpt(20, 110)
#define DPT_FanMode Dpt(20, 111)
#define DPT_MasterSlaveMode Dpt(20, 112)
#define DPT_StatusRoomSetp Dpt(20, 113)
#define DPT_ADAType Dpt(20, 120)
#define DPT_BackupMode Dpt(20, 121)
#define DPT_StartSynchronization Dpt(20, 122)
#define DPT_Behaviour_Lock_Unlock Dpt(20, 600)
#define DPT_Behaviour_Bus_Power_Up_Down Dpt(20, 601)
#define DPT_DALI_Fade_Time Dpt(20, 602)
#define DPT_BlinkingMode Dpt(20, 603)
#define DPT_LightControlMode Dpt(20, 604)
#define DPT_SwitchPBModel Dpt(20, 605)
#define DPT_PBAction Dpt(20, 606)
#define DPT_DimmPBModel Dpt(20, 607)
#define DPT_SwitchOnMode Dpt(20, 608)
#define DPT_LoadTypeSet Dpt(20, 609)
#define DPT_LoadTypeDetected Dpt(20, 610)
#define DPT_SABExceptBehaviour Dpt(20, 801)
#define DPT_SABBehaviour_Lock_Unlock Dpt(20, 802)
#define DPT_SSSBMode Dpt(20, 803)
#define DPT_BlindsControlMode Dpt(20, 804)
#define DPT_CommMode Dpt(20, 1000)
#define DPT_AddInfoTypes Dpt(20, 1001)
#define DPT_RF_ModeSelect Dpt(20, 1002)
#define DPT_RF_FilterSelect Dpt(20, 1003)
#define DPT_StatusGen Dpt(21, 1)
#define DPT_Device_Control Dpt(21, 2)
#define DPT_ForceSign Dpt(21, 100)
#define DPT_ForceSignCool Dpt(21, 101)
#define DPT_StatusRHC Dpt(21, 102)
#define DPT_StatusSDHWC Dpt(21, 103)
#define DPT_FuelTypeSet Dpt(21, 104)
#define DPT_StatusRCC Dpt(21, 105)
#define DPT_StatusAHU Dpt(21, 106)
#define DPT_LightActuatorErrorInfo Dpt(21, 601)
#define DPT_RF_ModeInfo Dpt(21, 1000)
#define DPT_RF_FilterInfo Dpt(21, 1001)
#define DPT_Channel_Activation_8 Dpt(21, 1010)
#define DPT_StatusDHWC Dpt(22, 100)
#define DPT_StatusRHCC Dpt(22, 101)
#define DPT_Media Dpt(22, 1000)
#define DPT_Channel_Activation_16 Dpt(22, 1010)
#define DPT_OnOff_Action Dpt(23, 1)
#define DPT_Alarm_Reaction Dpt(23, 2)
#define DPT_UpDown_Action Dpt(23, 3)
#define DPT_HVAC_PB_Action Dpt(23, 102)
#define DPT_VarString_8859_1 Dpt(24, 1)
#define DPT_DoubleNibble Dpt(25, 1000)
#define DPT_SceneInfo Dpt(26, 1)
#define DPT_CombinedInfoOnOff Dpt(27, 1)
#define DPT_UTF_8 Dpt(28, 1)
#define DPT_ActiveEnergy_V64 Dpt(29, 10)
#define DPT_ApparantEnergy_V64 Dpt(29, 11)
#define DPT_ReactiveEnergy_V64 Dpt(29, 12)
#define DPT_Channel_Activation_24 Dpt(30, 1010)
#define DPT_PB_Action_HVAC_Extended Dpt(31, 101)
#define DPT_Heat_Cool_Z Dpt(200, 100)
#define DPT_BinaryValue_Z Dpt(200, 101)
#define DPT_HVACMode_Z Dpt(201, 100)
#define DPT_DHWMode_Z Dpt(201, 102)
#define DPT_HVACContrMode_Z Dpt(201, 104)
#define DPT_EnablH_Cstage_Z Dpt(201, 105)
#define DPT_BuildingMode_Z Dpt(201, 107)
#define DPT_OccMode_Z Dpt(201, 108)
#define DPT_HVACEmergMode_Z Dpt(201, 109)
#define DPT_RelValue_Z Dpt(202, 1)
#define DPT_UCountValue8_Z Dpt(202, 2)
#define DPT_TimePeriodMsec_Z Dpt(203, 2)
#define DPT_TimePeriod10Msec_Z Dpt(203, 3)
#define DPT_TimePeriod100Msec_Z Dpt(203, 4)
#define DPT_TimePeriodSec_Z Dpt(203, 5)
#define DPT_TimePeriodMin_Z Dpt(203, 6)
#define DPT_TimePeriodHrs_Z Dpt(203, 7)
#define DPT_UFlowRateLiter_per_h_Z Dpt(203, 11)
#define DPT_UCountValue16_Z Dpt(203, 12)
#define DPT_UElCurrent_Z Dpt(203, 13)
#define DPT_PowerKW_Z Dpt(203, 14)
#define DPT_AtmPressureAbs_Z Dpt(203, 15)
#define DPT_PercentU16_Z Dpt(203, 17)
#define DPT_HVACAirQual_Z Dpt(203, 100)
#define DPT_WindSpeed_Z Dpt(203, 101)
#define DPT_SunIntensity_Z Dpt(203, 102)
#define DPT_HVACAirFlowAbs_Z Dpt(203, 104)
#define DPT_RelSignedValue_Z Dpt(204, 1)
#define DPT_DeltaTimeMsec_Z Dpt(205, 2)
#define DPT_DeltaTime10Msec_Z Dpt(205, 3)
#define DPT_DeltaTime100Msec_Z Dpt(205, 4)
#define DPT_DeltaTimeSec_Z Dpt(205, 5)
#define DPT_DeltaTimeMin_Z Dpt(205, 6)
#define DPT_DeltaTimeHrs_Z Dpt(205, 7)
#define DPT_Percent_V16_Z Dpt(205, 17)
#define DPT_TempHVACAbs_Z Dpt(205, 100)
#define DPT_TempHVACRel_Z Dpt(205, 101)
#define DPT_HVACAirFlowRel_Z Dpt(205, 102)
#define DPT_HVACModeNext Dpt(206, 100)
#define DPT_DHWModeNext Dpt(206, 102)
#define DPT_OccModeNext Dpt(206, 104)
#define DPT_BuildingModeNext Dpt(206, 105)
#define DPT_StatusBUC Dpt(207, 100)
#define DPT_LockSign Dpt(207, 101)
#define DPT_ValueDemBOC Dpt(207, 102)
#define DPT_ActPosDemAbs Dpt(207, 104)
#define DPT_StatusAct Dpt(207, 105)
#define DPT_StatusLightingActuator Dpt(207, 600)
#define DPT_StatusHPM Dpt(209, 100)
#define DPT_TempRoomDemAbs Dpt(209, 101)
#define DPT_StatusCPM Dpt(209, 102)
#define DPT_StatusWTC Dpt(209, 103)
#define DPT_TempFlowWaterDemAbs Dpt(210, 100)
#define DPT_EnergyDemWater Dpt(211, 100)
#define DPT_TempRoomSetpSetShift_3 Dpt(212, 100)
#define DPT_TempRoomSetpSet_3 Dpt(212, 101)
#define DPT_TempRoomSetpSet_4 Dpt(213, 100)
#define DPT_TempDHWSetpSet_4 Dpt(213, 101)
#define DPT_TempRoomSetpSetShift_4 Dpt(213, 102)
#define DPT_PowerFlowWaterDemHPM Dpt(214, 100)
#define DPT_PowerFlowWaterDemCPM Dpt(214, 101)
#define DPT_StatusBOC Dpt(215, 100)
#define DPT_StatusCC Dpt(215, 101)
#define DPT_SpecHeatProd Dpt(216, 100)
#define DPT_Version Dpt(217, 1)
#define DPT_VolumeLiter_Z Dpt(218, 1)
#define DPT_FlowRate_m3_per_h_Z Dpt(218, 2)
#define DPT_AlarmInfo Dpt(219, 1)
#define DPT_TempHVACAbsNext Dpt(220, 100)
#define DPT_SerNum Dpt(221, 1)
#define DPT_TempRoomSetpSetF16_3 Dpt(222, 100)
#define DPT_TempRoomSetpSetShiftF16_3 Dpt(222, 101)
#define DPT_EnergyDemAir Dpt(223, 100)
#define DPT_TempSupplyAirSetpSet Dpt(224, 100)
#define DPT_ScalingSpeed Dpt(225, 1)
#define DPT_Scaling_Step_Time Dpt(225, 2)
#define DPT_TariffNext Dpt(225, 3)
#define DPT_MeteringValue Dpt(229, 1)
#define DPT_MBus_Address Dpt(230, 1000)
#define DPT_Locale_ASCII Dpt(231, 1)
#define DPT_Colour_RGB Dpt(232, 600)
#define DPT_LanguageCodeAlpha2_ASCII Dpt(234, 1)
#define DPT_RegionCodeAlpha2_ASCII Dpt(234, 2)
#define DPT_Tariff_ActiveEnergy Dpt(235, 1)
#define DPT_Prioritised_Mode_Control Dpt(236, 1)
#define DPT_DALI_Control_Gear_Diagnostic Dpt(237, 600)
#define DPT_SceneConfig Dpt(238, 1)
#define DPT_DALI_Diagnostics Dpt(238, 600)
#define DPT_FlaggedScaling Dpt(239, 1)
#define DPT_CombinedPosition Dpt(240, 800)
#define DPT_StatusSAB Dpt(241, 800)
#define DPT_Colour_RGBW Dpt(251, 600)

    /**
     * This class represents a value to sent to or receive from the bus. The stored data is always a valid representation of the Dpt.
     */
    class Dpt
    {
        public:
            virtual ~Dpt() {}
            unsigned short mainGroup;
            unsigned short subGroup;
            unsigned short index;

            /**
             * Size of the Dpt. It can only assigned to or received from group objects with a matching size code.
             */
            virtual Go_SizeCode size() const = 0;

            /**
             * Encode the data to a byte array. The size of the array is implict decided by the size of the Dpt.
             */
            virtual void encode(uint8_t* data) const = 0;

            /**
             * Decodes the data from a byte array. @return true if the data can be decoded without error and false otherwise.
             * This method will also return false, if the date could fit the datatype but is invalid for this Dpt.
             */
            virtual bool decode(uint8_t* data) = 0;
    };

    template<typename T> class ValueDpt: public Dpt
    {
        public:
            ValueDpt() {};
            ValueDpt(T value)
            {
                _value = value;
            }

            virtual void value(T value)
            {
                _value = value;
            }

            T value() const
            {
                return _value;
            }

            operator T() const
            {
                return _value;
            }

            ValueDpt& operator=(const T value)
            {
                _value = value;
                return *this;
            }
        protected:
            T _value;
    };
}