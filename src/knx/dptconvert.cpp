#include "dptconvert.h"
#include "bits.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

#define ASSERT_PAYLOAD(x)      \
    if (payload_length != (x)) \
    return false
#define ENSURE_PAYLOAD(x) 


int KNX_Decode_Value(uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    if (payload_length > 0)
    {
        // DPT 1.* - Binary
        if (datatype.mainGroup == 1 && datatype.subGroup >= 1 && datatype.subGroup <= 23 && datatype.subGroup != 20 && !datatype.index)
        {
            return busValueToBinary(payload, payload_length, datatype, value);
        } // DPT 2.* - Binary Control
        if (datatype.mainGroup == 2 && datatype.subGroup >= 1 && datatype.subGroup <= 12 && datatype.index <= 1)
            return busValueToBinaryControl(payload, payload_length, datatype, value);
        // DPT 3.* - Step Control
        if (datatype.mainGroup == 3 && datatype.subGroup >= 7 && datatype.subGroup <= 8 && datatype.index <= 1)
            return busValueToStepControl(payload, payload_length, datatype, value);
        // DPT 4.* - Character// DPT 2.* - Binary Control
        if (datatype.mainGroup == 2 && datatype.subGroup >= 1 && datatype.subGroup <= 12 && datatype.index <= 1)
            return busValueToBinaryControl(payload, payload_length, datatype, value);
        // DPT 3.* - Step Control
        if (datatype.mainGroup == 3 && datatype.subGroup >= 7 && datatype.subGroup <= 8 && datatype.index <= 1)
            return busValueToStepControl(payload, payload_length, datatype, value);
        // DPT 4.* - Character
        if (datatype.mainGroup == 4 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && !datatype.index)
            return busValueToCharacter(payload, payload_length, datatype, value);
        // DPT 5.* - Unsigned 8 Bit Integer
        if (datatype.mainGroup == 5 && ((datatype.subGroup >= 1 && datatype.subGroup <= 6 && datatype.subGroup != 2) || datatype.subGroup == 10) && !datatype.index)
            return busValueToUnsigned8(payload, payload_length, datatype, value);
        // DPT 6.001/6.010 - Signed 8 Bit Integer
        if (datatype.mainGroup == 6 && (datatype.subGroup == 1 || datatype.subGroup == 10) && !datatype.index)
            return busValueToSigned8(payload, payload_length, datatype, value);
        // DPT 6.020 - Status with Mode
        if (datatype.mainGroup == 6 && datatype.subGroup == 20 && datatype.index <= 5)
            return busValueToStatusAndMode(payload, payload_length, datatype, value);
        // DPT 7.001/7.010/7.011/7.012/7.013 - Unsigned 16 Bit Integer
        if (datatype.mainGroup == 7 && (datatype.subGroup == 1 || (datatype.subGroup >= 10 && datatype.subGroup <= 13)) && !datatype.index)
            return busValueToUnsigned16(payload, payload_length, datatype, value);
        // DPT 7.002-DPT 7.007 - Time Period
        if (datatype.mainGroup == 7 && datatype.subGroup >= 2 && datatype.subGroup <= 7 && !datatype.index)
            return busValueToTimePeriod(payload, payload_length, datatype, value);
        // DPT 8.001/8.010/8.011 - Signed 16 Bit Integer
        if (datatype.mainGroup == 8 && (datatype.subGroup == 1 || datatype.subGroup == 10 || datatype.subGroup == 11) && !datatype.index)
            return busValueToSigned16(payload, payload_length, datatype, value);
        // DPT 8.002-DPT 8.007 - Time Delta
        if (datatype.mainGroup == 8 && datatype.subGroup >= 2 && datatype.subGroup <= 7 && !datatype.index)
            return busValueToTimeDelta(payload, payload_length, datatype, value);
        // DPT 9.* - 16 Bit Float
        if (datatype.mainGroup == 9 && ((datatype.subGroup >= 1 && datatype.subGroup <= 11 && datatype.subGroup != 9) || (datatype.subGroup >= 20 && datatype.subGroup <= 28)) && !datatype.index)
            return busValueToFloat16(payload, payload_length, datatype, value);
        // DPT 10.* - Time and Weekday
        if (datatype.mainGroup == 10 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToTime(payload, payload_length, datatype, value);
        // DPT 11.* - Date
        if (datatype.mainGroup == 11 && datatype.subGroup == 1 && !datatype.index)
            return busValueToDate(payload, payload_length, datatype, value);
        // DPT 12.* - Unsigned 32 Bit Integer
        if (datatype.mainGroup == 12 && datatype.subGroup == 1 && !datatype.index)
            return busValueToUnsigned32(payload, payload_length, datatype, value);
        // DPT 13.001/13.002/13.010-13.015 - Signed 32 Bit Integer
        if (datatype.mainGroup == 13 && (datatype.subGroup == 1 || datatype.subGroup == 2 || (datatype.subGroup >= 10 && datatype.subGroup <= 15)) && !datatype.index)
            return busValueToSigned32(payload, payload_length, datatype, value);
        // DPT 13.100 - Long Time Period
        if (datatype.mainGroup == 13 && datatype.subGroup == 100 && !datatype.index)
            return busValueToLongTimePeriod(payload, payload_length, datatype, value);
        // DPT 14.* - 32 Bit Float
        if (datatype.mainGroup == 14 && datatype.subGroup <= 79 && !datatype.index)
            return busValueToFloat32(payload, payload_length, datatype, value);
        // DPT 15.* - Access Data
        if (datatype.mainGroup == 15 && !datatype.subGroup && datatype.index <= 5)
            return busValueToAccess(payload, payload_length, datatype, value);
        // DPT 16.* - String
        if (datatype.mainGroup == 16 && datatype.subGroup <= 1 && !datatype.index)
            return busValueToString(payload, payload_length, datatype, value);
        // DPT 17.* - Scene Number
        if (datatype.mainGroup == 17 && datatype.subGroup == 1 && !datatype.index)
            return busValueToScene(payload, payload_length, datatype, value);
        // DPT 18.* - Scene Control
        if (datatype.mainGroup == 18 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToSceneControl(payload, payload_length, datatype, value);
        // DPT 19.* - Date and Time
        if (datatype.mainGroup == 19 && datatype.subGroup == 1 && (datatype.index <= 3 || datatype.index == 9 || datatype.index == 10))
            return busValueToDateTime(payload, payload_length, datatype, value);
        // DPT 26.* - Scene Info
        if (datatype.mainGroup == 26 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToSceneInfo(payload, payload_length, datatype, value);
        // DPT 28.* - Unicode String
        if (datatype.mainGroup == 28 && datatype.subGroup == 1 && !datatype.index)
            return busValueToUnicode(payload, payload_length, datatype, value);
        // DPT 29.* - Signed 64 Bit Integer
        if (datatype.mainGroup == 29 && datatype.subGroup >= 10 && datatype.subGroup <= 12 && !datatype.index)
            return busValueToSigned64(payload, payload_length, datatype, value);
        // DPT 219.* - Alarm Info
        if (datatype.mainGroup == 219 && datatype.subGroup == 1 && datatype.index <= 10)
            return busValueToAlarmInfo(payload, payload_length, datatype, value);
        // DPT 221.* - Serial Number
        if (datatype.mainGroup == 221 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToSerialNumber(payload, payload_length, datatype, value);
        // DPT 217.* - Version
        if (datatype.mainGroup == 217 && datatype.subGroup == 1 && datatype.index <= 2)
            return busValueToVersion(payload, payload_length, datatype, value);
        // DPT 225.001/225.002 - Scaling Speed and Scaling Step Time
        if (datatype.mainGroup == 225 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && datatype.index <= 1)
            return busValueToScaling(payload, payload_length, datatype, value);
        // DPT 225.003 - Next Tariff
        if (datatype.mainGroup == 225 && datatype.subGroup == 3 && datatype.index <= 1)
            return busValueToTariff(payload, payload_length, datatype, value);
        // DPT 231.* - Locale
        if (datatype.mainGroup == 231 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToLocale(payload, payload_length, datatype, value);
        // DPT 232.600 - RGB
        if (datatype.mainGroup == 232 && datatype.subGroup == 600 && !datatype.index)
            return busValueToRGB(payload, payload_length, datatype, value);
        // DPT 234.* - Language and Region
        if (datatype.mainGroup == 234 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && !datatype.index)
            return busValueToLocale(payload, payload_length, datatype, value);
        // DPT 235.* - Active Energy
        if (datatype.mainGroup == 235 && datatype.subGroup == 1 && datatype.index <= 3)
            return busValueToActiveEnergy(payload, payload_length, datatype, value);
        // DPT 238.* - Scene Config
        if (datatype.mainGroup == 238 && datatype.subGroup == 1 && datatype.index <= 2)
            return busValueToSceneConfig(payload, payload_length, datatype, value);
        // DPT 239.* - Flagged Scaling
        if (datatype.mainGroup == 239 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToFlaggedScaling(payload, payload_length, datatype, value);
        if (datatype.mainGroup == 4 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && !datatype.index)
            return busValueToCharacter(payload, payload_length, datatype, value);
        // DPT 5.* - Unsigned 8 Bit Integer
        if (datatype.mainGroup == 5 && ((datatype.subGroup >= 1 && datatype.subGroup <= 6 && datatype.subGroup != 2) || datatype.subGroup == 10) && !datatype.index)
            return busValueToUnsigned8(payload, payload_length, datatype, value);
        // DPT 6.001/6.010 - Signed 8 Bit Integer
        if (datatype.mainGroup == 6 && (datatype.subGroup == 1 || datatype.subGroup == 10) && !datatype.index)
            return busValueToSigned8(payload, payload_length, datatype, value);
        // DPT 6.020 - Status with Mode
        if (datatype.mainGroup == 6 && datatype.subGroup == 20 && datatype.index <= 5)
            return busValueToStatusAndMode(payload, payload_length, datatype, value);
        // DPT 7.001/7.010/7.011/7.012/7.013 - Unsigned 16 Bit Integer
        if (datatype.mainGroup == 7 && (datatype.subGroup == 1 || (datatype.subGroup >= 10 && datatype.subGroup <= 13)) && !datatype.index)
            return busValueToUnsigned16(payload, payload_length, datatype, value);
        // DPT 7.002-DPT 7.007 - Time Period
        if (datatype.mainGroup == 7 && datatype.subGroup >= 2 && datatype.subGroup <= 7 && !datatype.index)
            return busValueToTimePeriod(payload, payload_length, datatype, value);
        // DPT 8.001/8.010/8.011 - Signed 16 Bit Integer
        if (datatype.mainGroup == 8 && (datatype.subGroup == 1 || datatype.subGroup == 10 || datatype.subGroup == 11) && !datatype.index)
            return busValueToSigned16(payload, payload_length, datatype, value);
        // DPT 8.002-DPT 8.007 - Time Delta
        if (datatype.mainGroup == 8 && datatype.subGroup >= 2 && datatype.subGroup <= 7 && !datatype.index)
            return busValueToTimeDelta(payload, payload_length, datatype, value);
        // DPT 9.* - 16 Bit Float
        if (datatype.mainGroup == 9 && ((datatype.subGroup >= 1 && datatype.subGroup <= 11 && datatype.subGroup != 9) || (datatype.subGroup >= 20 && datatype.subGroup <= 28)) && !datatype.index)
            return busValueToFloat16(payload, payload_length, datatype, value);
        // DPT 10.* - Time and Weekday
        if (datatype.mainGroup == 10 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToTime(payload, payload_length, datatype, value);
        // DPT 11.* - Date
        if (datatype.mainGroup == 11 && datatype.subGroup == 1 && !datatype.index)
            return busValueToDate(payload, payload_length, datatype, value);
        // DPT 12.* - Unsigned 32 Bit Integer
        if (datatype.mainGroup == 12 && datatype.subGroup == 1 && !datatype.index)
            return busValueToUnsigned32(payload, payload_length, datatype, value);
        // DPT 13.001/13.002/13.010-13.015 - Signed 32 Bit Integer
        if (datatype.mainGroup == 13 && (datatype.subGroup == 1 || datatype.subGroup == 2 || (datatype.subGroup >= 10 && datatype.subGroup <= 15)) && !datatype.index)
            return busValueToSigned32(payload, payload_length, datatype, value);
        // DPT 13.100 - Long Time Period
        if (datatype.mainGroup == 13 && datatype.subGroup == 100 && !datatype.index)
            return busValueToLongTimePeriod(payload, payload_length, datatype, value);
        // DPT 14.* - 32 Bit Float
        if (datatype.mainGroup == 14 && datatype.subGroup <= 79 && !datatype.index)
            return busValueToFloat32(payload, payload_length, datatype, value);
        // DPT 15.* - Access Data
        if (datatype.mainGroup == 15 && !datatype.subGroup && datatype.index <= 5)
            return busValueToAccess(payload, payload_length, datatype, value);
        // DPT 16.* - String
        if (datatype.mainGroup == 16 && datatype.subGroup <= 1 && !datatype.index)
            return busValueToString(payload, payload_length, datatype, value);
        // DPT 17.* - Scene Number
        if (datatype.mainGroup == 17 && datatype.subGroup == 1 && !datatype.index)
            return busValueToScene(payload, payload_length, datatype, value);
        // DPT 18.* - Scene Control
        if (datatype.mainGroup == 18 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToSceneControl(payload, payload_length, datatype, value);
        // DPT 19.* - Date and Time
        if (datatype.mainGroup == 19 && datatype.subGroup == 1 && (datatype.index <= 3 || datatype.index == 9 || datatype.index == 10))
            return busValueToDateTime(payload, payload_length, datatype, value);
        // DPT 26.* - Scene Info
        if (datatype.mainGroup == 26 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToSceneInfo(payload, payload_length, datatype, value);
        // DPT 28.* - Unicode String
        if (datatype.mainGroup == 28 && datatype.subGroup == 1 && !datatype.index)
            return busValueToUnicode(payload, payload_length, datatype, value);
        // DPT 29.* - Signed 64 Bit Integer
        if (datatype.mainGroup == 29 && datatype.subGroup >= 10 && datatype.subGroup <= 12 && !datatype.index)
            return busValueToSigned64(payload, payload_length, datatype, value);
        // DPT 219.* - Alarm Info
        if (datatype.mainGroup == 219 && datatype.subGroup == 1 && datatype.index <= 10)
            return busValueToAlarmInfo(payload, payload_length, datatype, value);
        // DPT 221.* - Serial Number
        if (datatype.mainGroup == 221 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToSerialNumber(payload, payload_length, datatype, value);
        // DPT 217.* - Version
        if (datatype.mainGroup == 217 && datatype.subGroup == 1 && datatype.index <= 2)
            return busValueToVersion(payload, payload_length, datatype, value);
        // DPT 225.001/225.002 - Scaling Speed and Scaling Step Time
        if (datatype.mainGroup == 225 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && datatype.index <= 1)
            return busValueToScaling(payload, payload_length, datatype, value);
        // DPT 225.003 - Next Tariff
        if (datatype.mainGroup == 225 && datatype.subGroup == 3 && datatype.index <= 1)
            return busValueToTariff(payload, payload_length, datatype, value);
        // DPT 231.* - Locale
        if (datatype.mainGroup == 231 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToLocale(payload, payload_length, datatype, value);
        // DPT 232.600 - RGB
        if (datatype.mainGroup == 232 && datatype.subGroup == 600 && !datatype.index)
            return busValueToRGB(payload, payload_length, datatype, value);
        // DPT 234.* - Language and Region
        if (datatype.mainGroup == 234 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && !datatype.index)
            return busValueToLocale(payload, payload_length, datatype, value);
        // DPT 235.* - Active Energy
        if (datatype.mainGroup == 235 && datatype.subGroup == 1 && datatype.index <= 3)
            return busValueToActiveEnergy(payload, payload_length, datatype, value);
        // DPT 238.* - Scene Config
        if (datatype.mainGroup == 238 && datatype.subGroup == 1 && datatype.index <= 2)
            return busValueToSceneConfig(payload, payload_length, datatype, value);
        // DPT 239.* - Flagged Scaling
        if (datatype.mainGroup == 239 && datatype.subGroup == 1 && datatype.index <= 1)
            return busValueToFlaggedScaling(payload, payload_length, datatype, value);
    }
    return false;
}

int KNX_Encode_Value(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{

    if (datatype.mainGroup == 1 && datatype.subGroup >= 1 && datatype.subGroup <= 23 && datatype.subGroup != 20 && !datatype.index)
        return valueToBusValueBinary(value, payload, payload_length, datatype);
    // DPT 2.* - Binary Control
    if (datatype.mainGroup == 2 && datatype.subGroup >= 1 && datatype.subGroup <= 12 && datatype.index <= 1)
        return valueToBusValueBinaryControl(value, payload, payload_length, datatype);
    // DPT 3.* - Step Control
    if (datatype.mainGroup == 3 && datatype.subGroup >= 7 && datatype.subGroup <= 8 && datatype.index <= 1)
        return valueToBusValueStepControl(value, payload, payload_length, datatype);
    // DPT 4.* - Character
    if (datatype.mainGroup == 4 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && !datatype.index)
        return valueToBusValueCharacter(value, payload, payload_length, datatype);
    // DPT 5.* - Unsigned 8 Bit Integer
    if (datatype.mainGroup == 5 && ((datatype.subGroup >= 1 && datatype.subGroup <= 6 && datatype.subGroup != 2) || datatype.subGroup == 10) && !datatype.index)
        return valueToBusValueUnsigned8(value, payload, payload_length, datatype);
    // DPT 6.001/6.010 - Signed 8 Bit Integer
    if (datatype.mainGroup == 6 && (datatype.subGroup == 1 || datatype.subGroup == 10) && !datatype.index)
        return valueToBusValueSigned8(value, payload, payload_length, datatype);
    // DPT 6.020 - Status with Mode
    if (datatype.mainGroup == 6 && datatype.subGroup == 20 && datatype.index <= 5)
        return valueToBusValueStatusAndMode(value, payload, payload_length, datatype);
    // DPT 7.001/7.010/7.011/7.012/7.013 - Unsigned 16 Bit Integer
    if (datatype.mainGroup == 7 && (datatype.subGroup == 1 || (datatype.subGroup >= 10 && datatype.subGroup <= 13)) && !datatype.index)
        return valueToBusValueUnsigned16(value, payload, payload_length, datatype);
    // DPT 7.002-DPT 7.007 - Time Period
    if (datatype.mainGroup == 7 && datatype.subGroup >= 2 && datatype.subGroup <= 7 && !datatype.index)
        return valueToBusValueTimePeriod(value, payload, payload_length, datatype);
    // DPT 8.001/8.010/8.011 - Signed 16 Bit Integer
    if (datatype.mainGroup == 8 && (datatype.subGroup == 1 || datatype.subGroup == 10 || datatype.subGroup == 11) && !datatype.index)
        return valueToBusValueSigned16(value, payload, payload_length, datatype);
    // DPT 8.002-DPT 8.007 - Time Delta
    if (datatype.mainGroup == 8 && datatype.subGroup >= 2 && datatype.subGroup <= 7 && !datatype.index)
        return valueToBusValueTimeDelta(value, payload, payload_length, datatype);
    // DPT 9.* - 16 Bit Float
    if (datatype.mainGroup == 9 && ((datatype.subGroup >= 1 && datatype.subGroup <= 11 && datatype.subGroup != 9) || (datatype.subGroup >= 20 && datatype.subGroup <= 28)) && !datatype.index)
        return valueToBusValueFloat16(value, payload, payload_length, datatype);
    // DPT 10.* - Time and Weekday
    if (datatype.mainGroup == 10 && datatype.subGroup == 1 && datatype.index <= 1)
        return valueToBusValueTime(value, payload, payload_length, datatype);
    // DPT 11.* - Date
    if (datatype.mainGroup == 11 && datatype.subGroup == 1 && !datatype.index)
        return valueToBusValueDate(value, payload, payload_length, datatype);
    // DPT 12.* - Unsigned 32 Bit Integer
    if (datatype.mainGroup == 12 && datatype.subGroup == 1 && !datatype.index)
        return valueToBusValueUnsigned32(value, payload, payload_length, datatype);
    // DPT 13.001/13.002/13.010-13.015 - Signed 32 Bit Integer
    if (datatype.mainGroup == 13 && (datatype.subGroup == 1 || datatype.subGroup == 2 || (datatype.subGroup >= 10 && datatype.subGroup <= 15)) && !datatype.index)
        return valueToBusValueSigned32(value, payload, payload_length, datatype);
    // DPT 13.100 - Long Time Period
    if (datatype.mainGroup == 13 && datatype.subGroup == 100 && !datatype.index)
        return valueToBusValueLongTimePeriod(value, payload, payload_length, datatype);
    // DPT 14.* - 32 Bit Float
    if (datatype.mainGroup == 14 && datatype.subGroup <= 79 && !datatype.index)
        return valueToBusValueFloat32(value, payload, payload_length, datatype);
    // DPT 15.* - Access Data
    if (datatype.mainGroup == 15 && !datatype.subGroup && datatype.index <= 5)
        return valueToBusValueAccess(value, payload, payload_length, datatype);
    // DPT 16.* - String
    if (datatype.mainGroup == 16 && datatype.subGroup <= 1 && !datatype.index)
        return valueToBusValueString(value, payload, payload_length, datatype);
    // DPT 17.* - Scene Number
    if (datatype.mainGroup == 17 && datatype.subGroup == 1 && !datatype.index)
        return valueToBusValueScene(value, payload, payload_length, datatype);
    // DPT 18.* - Scene Control
    if (datatype.mainGroup == 18 && datatype.subGroup == 1 && datatype.index <= 1)
        return valueToBusValueSceneControl(value, payload, payload_length, datatype);
    // DPT 19.* - Date and Time
    if (datatype.mainGroup == 19 && datatype.subGroup == 1 && (datatype.index <= 3 || datatype.index == 9 || datatype.index == 10))
        return valueToBusValueDateTime(value, payload, payload_length, datatype);
    // DPT 26.* - Scene Info
    if (datatype.mainGroup == 26 && datatype.subGroup == 1 && datatype.index <= 1)
        return valueToBusValueSceneInfo(value, payload, payload_length, datatype);
    // DPT 28.* - Unicode String
    if (datatype.mainGroup == 28 && datatype.subGroup == 1 && !datatype.index)
        return valueToBusValueUnicode(value, payload, payload_length, datatype);
    // DPT 29.* - Signed 64 Bit Integer
    if (datatype.mainGroup == 29 && datatype.subGroup >= 10 && datatype.subGroup <= 12 && !datatype.index)
        return valueToBusValueSigned64(value, payload, payload_length, datatype);
    // DPT 219.* - Alarm Info
    if (datatype.mainGroup == 219 && datatype.subGroup == 1 && datatype.index <= 10)
        return valueToBusValueAlarmInfo(value, payload, payload_length, datatype);
    // DPT 221.* - Serial Number
    if (datatype.mainGroup == 221 && datatype.subGroup == 1 && datatype.index <= 1)
        return valueToBusValueSerialNumber(value, payload, payload_length, datatype);
    // DPT 217.* - Version
    if (datatype.mainGroup == 217 && datatype.subGroup == 1 && datatype.index <= 2)
        return valueToBusValueVersion(value, payload, payload_length, datatype);
    // DPT 225.001/225.002 - Scaling Speed and Scaling Step Time
    if (datatype.mainGroup == 225 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && datatype.index <= 1)
        return valueToBusValueScaling(value, payload, payload_length, datatype);
    // DPT 225.003 - Next Tariff
    if (datatype.mainGroup == 225 && datatype.subGroup == 3 && datatype.index <= 1)
        return valueToBusValueTariff(value, payload, payload_length, datatype);
    // DPT 231.* - Locale
    if (datatype.mainGroup == 231 && datatype.subGroup == 1 && datatype.index <= 1)
        return valueToBusValueLocale(value, payload, payload_length, datatype);
    // DPT 232.600 - RGB
    if (datatype.mainGroup == 232 && datatype.subGroup == 600 && !datatype.index)
        return valueToBusValueRGB(value, payload, payload_length, datatype);
    // DPT 234.* - Language and Region
    if (datatype.mainGroup == 234 && datatype.subGroup >= 1 && datatype.subGroup <= 2 && !datatype.index)
        return valueToBusValueLocale(value, payload, payload_length, datatype);
    // DPT 235.* - Active Energy
    if (datatype.mainGroup == 235 && datatype.subGroup == 1 && datatype.index <= 3)
        return valueToBusValueActiveEnergy(value, payload, payload_length, datatype);
    // DPT 238.* - Scene Config
    if (datatype.mainGroup == 238 && datatype.subGroup == 1 && datatype.index <= 2)
        return valueToBusValueSceneConfig(value, payload, payload_length, datatype);
    // DPT 239.* - Flagged Scaling
    if (datatype.mainGroup == 239 && datatype.subGroup == 1 && datatype.index <= 1)
        return valueToBusValueFlaggedScaling(value, payload, payload_length, datatype);
    return false;
}

int busValueToBinary(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    value = bitFromPayload(payload, 7);
    return true;
}

int busValueToBinaryControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    switch (datatype.index)
    {
        case 0:
            value = bitFromPayload(payload, 6);
            return true;
        case 1:
            value = bitFromPayload(payload, 7);
            return true;
    }

    return false;
}

int busValueToStepControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    switch (datatype.index)
    {
        case 0:
            value = bitFromPayload(payload, 4);
            return true;
        case 1:
        {
            const unsigned char stepCode = unsigned8FromPayload(payload, 0) & 0x07;
            value = stepCode;
            return true;
        }
    }

    return false;
}
int busValueToCharacter(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    int8_t charValue = signed8FromPayload(payload, 0);
    if (datatype.subGroup == 1 && (charValue & 0x80))
        return false;
    if (datatype.subGroup == 2)
    {
        value = (uint8_t)charValue;
        return true;
    }

    value = charValue;
    return true;
}

int busValueToUnsigned8(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    switch (datatype.subGroup)
    {
        case 1:
            value = (uint8_t)round(unsigned8FromPayload(payload, 0) * 100.0 / 255.0);
            return true;

        case 3:
            value = (uint8_t)round(unsigned8FromPayload(payload, 0) * 360.0 / 255.0);
            return true;

        case 6:
        {
            uint8_t numValue = unsigned8FromPayload(payload, 0);
            if (numValue == 0xFF)
                return false;
            value = numValue;
            return true;
        }
    }

    value = unsigned8FromPayload(payload, 0);
    return true;
}

int busValueToSigned8(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    value = (uint8_t)(unsigned8FromPayload(payload, 0));
    return true;
}

int busValueToStatusAndMode(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    if (datatype.index < 5)
    {
        value = bitFromPayload(payload, datatype.index);
        return true;
    }
    else if (datatype.index == 5)
    {
        value = (uint8_t)(unsigned8FromPayload(payload, 0) & 0x07);
        return true;
    }
    return false;
}

int busValueToUnsigned16(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);
    value = unsigned16FromPayload(payload, 0);
    return true;
}

int busValueToTimePeriod(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);

    int64_t duration = unsigned16FromPayload(payload, 0);
    value = duration;
    return true;
}

int busValueToSigned16(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);
    if (datatype.subGroup == 10)
    {
        value = signed16FromPayload(payload, 0) / 100.0;
        return true;
    }
    value = signed16FromPayload(payload, 0);
    return true;
}

int busValueToTimeDelta(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);

    int64_t duration = signed16FromPayload(payload, 0);
    value = duration;
    return true;
}

int busValueToFloat16(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);
    if (unsigned16FromPayload(payload, 0) == 0x7FFF)
        return false;

    value = float16FromPayload(payload, 0);
    return true;
}

int busValueToTime(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(3);
    switch (datatype.index)
    {
        case 0:
            value = (uint8_t)((unsigned8FromPayload(payload, 0) >> 5) & 0x07);
            return true;
        case 1:
        {
            unsigned char hours = unsigned8FromPayload(payload, 0) & 0x1F;
            unsigned char minutes = unsigned8FromPayload(payload, 1) & 0x3F;
            unsigned char seconds = unsigned8FromPayload(payload, 2) & 0x3F;

            if (hours > 23 || minutes > 59 || seconds > 59)
                return false;
            struct tm tmp = {0};
            tmp.tm_hour = hours;
            tmp.tm_min = minutes;
            tmp.tm_sec = seconds;
            value = tmp;
            return true;
        }
    }

    return false;
}

int busValueToDate(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(3);
    unsigned short year = unsigned8FromPayload(payload, 2) & 0x7F;
    unsigned char month = unsigned8FromPayload(payload, 1) & 0x0F;
    unsigned char day = unsigned8FromPayload(payload, 0) & 0x1F;

    if (year > 99 || month < 1 || month > 12 || day < 1)
        return false;

    struct tm tmp = {0};
    year += year >= 90 ? 1900 : 2000;
    tmp.tm_mday = day;
    tmp.tm_year = year;
    tmp.tm_mon = month;
    value = tmp;
    return true;
}

int busValueToUnsigned32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(4);
    value = unsigned32FromPayload(payload, 0);
    return true;
}

int busValueToSigned32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(4);
    value = signed32FromPayload(payload, 0);
    return true;
}

int busValueToLongTimePeriod(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(4);
    value = signed32FromPayload(payload, 0);
    return true;
}

int busValueToFloat32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(4);
    value = float32FromPayload(payload, 0);
    return true;
}

int busValueToAccess(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(4);
    switch (datatype.index)
    {
        case 0:
        {
            int32_t digits = 0;
            for (int n = 0, factor = 100000; n < 6; ++n, factor /= 10)
            {
                unsigned char digit = bcdFromPayload(payload, n);
                if (digit > 9)
                    return false;
                digits += digit * factor;
            }
            value = digits;
            return true;
        }
        case 1:
        case 2:
        case 3:
        case 4:
            value = bitFromPayload(payload, 23 + datatype.index);
            return true;
        case 5:
            value = bcdFromPayload(payload, 7);
            return true;
    }

    return false;
}

int busValueToString(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(14);
    char strValue[15];
    strValue[14] = '\0';
    for (int n = 0; n < 14; ++n)
    {
        strValue[n] = signed8FromPayload(payload, n);
        if (!datatype.subGroup && (strValue[n] & 0x80))
            return false;
    }
    value = strValue;
    return true;
}

int busValueToScene(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    value = (uint8_t)(unsigned8FromPayload(payload, 0) & 0x3F);
    return true;
}

int busValueToSceneControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    switch (datatype.index)
    {
        case 0:
        {
            value = bitFromPayload(payload, 0);
            return true;
        }
        case 1:
        {
            value = (uint8_t)(unsigned8FromPayload(payload, 0) & 0x3F);
            return true;
        }
    }

    return false;
}

int busValueToSceneInfo(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    switch (datatype.index)
    {
        case 0:
        {
            value = bitFromPayload(payload, 1);
            return true;
        }
        case 1:
        {
            value = (uint8_t)(unsigned8FromPayload(payload, 0) & 0x3F);
            return true;
        }
    }

    return false;
}

int busValueToSceneConfig(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(1);
    switch (datatype.index)
    {
        case 0:
        {
            value = (uint8_t)(unsigned8FromPayload(payload, 0) & 0x3F);
            return true;
        }
        case 1:
        case 2:
        {
            value = bitFromPayload(payload, 2 - datatype.index);
            return true;
        }
    }

    return false;
}

int busValueToDateTime(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(8);
    if (datatype.index == 3)
    {
        value = bitFromPayload(payload, 48);
        return true;
    }

    if (!bitFromPayload(payload, 48))
    {
        switch (datatype.index)
        {
            case 0:
            {
                if (bitFromPayload(payload, 51) || bitFromPayload(payload, 52))
                    return false;

                unsigned short year = unsigned8FromPayload(payload, 0) + 1900;
                unsigned short month = unsigned8FromPayload(payload, 1) & 0x0F;
                unsigned short day = unsigned8FromPayload(payload, 2) & 0x1F;
                unsigned short hours = unsigned8FromPayload(payload, 3) & 0x1F;
                unsigned short minutes = unsigned8FromPayload(payload, 4) & 0x3F;
                unsigned short seconds = unsigned8FromPayload(payload, 5) & 0x3F;

                if ((month < 1 || month > 12 || day < 1))
                    return false;
                if ((hours > 24 || minutes > 59 || seconds > 59))
                    return false;

                struct tm tmp = {0};
                tmp.tm_sec = seconds;
                tmp.tm_min = minutes;
                tmp.tm_hour = hours;
                tmp.tm_mday = day;
                tmp.tm_mon = month;
                tmp.tm_year = year;
                value = tmp;
                return true;
            }
            case 1:
            {
                if (bitFromPayload(payload, 53))
                    return false;

                value = (uint8_t)((unsigned8FromPayload(payload, 3) >> 5) & 0x07);
                return true;
            }
            case 2:
            {
                if (bitFromPayload(payload, 50))
                    return false;

                value = bitFromPayload(payload, 49);
                return true;
            }
            case 9:
            {
                value = bitFromPayload(payload, 55);
                return true;
            }
            case 10:
            {
                value = bitFromPayload(payload, 56);
                return true;
            }
        }
    }

    return false;
}

int busValueToUnicode(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    //TODO
    return false;
}

int busValueToSigned64(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(8);
    value = signed64FromPayload(payload, 0);
    return true;
}

int busValueToAlarmInfo(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(6);
    switch (datatype.index)
    {
        case 1:
        {
            unsigned char prio = unsigned8FromPayload(payload, 1);
            if (prio > 3)
                return false;
            value = prio;
            return true;
        }
        case 0:
        case 2:
        case 3:
            value = unsigned8FromPayload(payload, datatype.index);
            return true;
        case 4:
        case 5:
        case 6:
        case 7:
            value = bitFromPayload(payload, 43 - datatype.index);
            return true;
        case 8:
        case 9:
        case 10:
            value = bitFromPayload(payload, 55 - datatype.index);
            return true;
    }
    return false;
}

int busValueToSerialNumber(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(6);
    switch (datatype.index)
    {
        case 0:
            value = unsigned16FromPayload(payload, 0);
            return true;
        case 1:
            value = unsigned32FromPayload(payload, 2);
            return true;
    }
    return false;
}

int busValueToVersion(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);
    switch (datatype.index)
    {
        case 0:
            value = (uint8_t)((unsigned8FromPayload(payload, 0) >> 3) & 0x1F);
            return true;
        case 1:
            value = (uint16_t)((unsigned16FromPayload(payload, 0) >> 6) & 0x1F);
            return true;
        case 2:
            value = (uint8_t)(unsigned8FromPayload(payload, 1) & 0x3F);
            return true;
    }

    return false;
}

int busValueToScaling(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(3);
    switch (datatype.index)
    {
        case 0:
            value = unsigned16FromPayload(payload, 0);
            return true;
        case 1:
            value = (uint8_t)(unsigned8FromPayload(payload, 2) * 100.0 / 255.0);
            return true;
    }

    return false;
}

int busValueToTariff(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(3);
    switch (datatype.index)
    {
        case 0:
            value = unsigned16FromPayload(payload, 0);
            return true;
        case 1:
        {
            uint8_t tariff = unsigned8FromPayload(payload, 2);
            if (tariff > 254)
                return false;
            value = tariff;
            return true;
        }
    }

    return false;
}

int busValueToLocale(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(datatype.mainGroup == 231 ? 4 : 2);
    if (!datatype.index || (datatype.mainGroup == 231 && datatype.index == 1))
    {
        char code[2];
        code[0] = unsigned8FromPayload(payload, datatype.index * 2);
        code[1] = unsigned8FromPayload(payload, datatype.index * 2 + 1);
        value = code;
        return true;
    }
    return false;
}

int busValueToRGB(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(3);
    uint32_t rgb = unsigned16FromPayload(payload, 0) * 256 + unsigned8FromPayload(payload, 2);
    if (rgb > 16777215)
        return false;
    value = rgb;
    return true;
}

int busValueToFlaggedScaling(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(2);
    switch (datatype.index)
    {
        case 0:
            value = (uint8_t)(unsigned8FromPayload(payload, 0) * 100.0 / 255.0);
            return true;
        case 1:
            value = bitFromPayload(payload, 15);
            return true;
    }
    return false;
}

int busValueToActiveEnergy(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value)
{
    ASSERT_PAYLOAD(6);
    switch (datatype.index)
    {
        case 0:
            value = signed32FromPayload(payload, 0);
            return true;
        case 1:
            value = unsigned8FromPayload(payload, 4);
            return true;
        case 2:
        case 3:
            value = bitFromPayload(payload, datatype.index + 44);
            return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------------------------------------------

int valueToBusValueBinary(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    bitToPayload(payload, payload_length, 7, value);
    return true;
}

int valueToBusValueBinaryControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
            bitToPayload(payload, payload_length, 6, value);
            break;
        case 1:
            bitToPayload(payload, payload_length, 7, value);
            break;
        default:
            return false;
    }

    return true;
}

int valueToBusValueStepControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
            bitToPayload(payload, payload_length, 4, value);
            break;
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(7))
                return false;
            unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value, 0x07);
        }
        break;
        default:
            return false;
    }

    return true;
}

int valueToBusValueCharacter(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((uint64_t)value < INT64_C(0) || (uint64_t)value > INT64_C(255) || (datatype.subGroup == 1 && (uint64_t)value > INT64_C(127)))
        return false;
    unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFF);
    return true;
}

int valueToBusValueUnsigned8(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(0))
        return false;

    switch (datatype.subGroup)
    {
        case 1:
        {
            if ((double)value > 100.0)
                return false;
            unsigned8ToPayload(payload, payload_length, 0, round((double)value * 255.0 / 100.0), 0xFF);
            break;
        }
        case 3:
        {
            if ((double)value > 360.0)
                return false;
            unsigned8ToPayload(payload, payload_length, 0, round((double)value * 255.0 / 360.0), 0xFF);
            break;
        }
        case 6:
        {
            if ((int64_t)value > INT64_C(254))
                return false;
            unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFF);
            break;
        }
        default:
        {
            if ((int64_t)value > INT64_C(255))
                return false;
            unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFF);
        }
    }

    return true;
}

int valueToBusValueSigned8(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(-128) || (int64_t)value > INT64_C(127))
        return false;

    signed8ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFF);
    return true;
}

int valueToBusValueStatusAndMode(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if (datatype.index < 5)
        bitToPayload(payload, payload_length, datatype.index, value);
    else if (datatype.index == 5)
    {
        if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(7))
            return false;
        unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value, 0x07);
    }
    else
        return false;

    return true;
}

int valueToBusValueUnsigned16(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(65535))
        return false;

    unsigned16ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFFFF);
    return true;
}

int valueToBusValueTimePeriod(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    struct tm tmp = value;
    time_t timeSinceEpoch = mktime(&tmp);

    if (timeSinceEpoch < INT64_C(0) || timeSinceEpoch > INT64_C(65535))
        return false;

    unsigned16ToPayload(payload, payload_length, 0, timeSinceEpoch, 0xFFFF);
    return true;
}

int valueToBusValueSigned16(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(-32768) || (int64_t)value > INT64_C(32767))
        return false;

    if (datatype.subGroup == 10)
    {
        if ((double)value < -327.68 || (double)value > 327.67)
            return false;
        signed16ToPayload(payload, payload_length, 0, (double)value * 100.0, 0xFF);
    }
    else
        signed16ToPayload(payload, payload_length, 0, (uint64_t)value, 0xffff);

    return true;
}

int valueToBusValueTimeDelta(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    struct tm tmp = value;
    time_t timeSinceEpoch = mktime(&tmp);

    if (timeSinceEpoch < INT64_C(-32768) || timeSinceEpoch > INT64_C(32767))
        return false;

    signed16ToPayload(payload, payload_length, 0, timeSinceEpoch, 0xFFFF);
    return true;
}

int valueToBusValueFloat16(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    double numValue = value;

    // bigger values like 670760.0 result in 0x7FFF which denotes invalid data.
    // I'm not sure if the GO shouldn't be updated to this value instead
    if (numValue > 670433.28)
        return false;

    if (numValue < -671088.64)
        return false;

    switch (datatype.subGroup)
    {
        case 1:
            if (numValue < -273.0)
                return false;
            break;
        case 2:
        case 3:
        case 10:
        case 11:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            if (numValue < -670760.0)
                return false;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 28:
            if (numValue < 0.0)
                return false;
            break;
        case 27:
            if (numValue < -459.6)
                return false;
            break;
    }

    float16ToPayload(payload, payload_length, 0, numValue, 0xFFFF);
    return true;
}

int valueToBusValueTime(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(7))
                return false;
            ENSURE_PAYLOAD(3);
            unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value << 5, 0xE0);
            break;
        }
        case 1:
        {
            struct tm tmp = value;
            unsigned8ToPayload(payload, payload_length, 0, tmp.tm_hour, 0x1F);
            unsigned8ToPayload(payload, payload_length, 1, tmp.tm_min, 0x3F);
            unsigned8ToPayload(payload, payload_length, 2, tmp.tm_sec, 0x3F);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueDate(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    struct tm tmp = value;
    if (tmp.tm_year < 1990 || tmp.tm_year > 2089)
        return false;

    unsigned8ToPayload(payload, payload_length, 0, tmp.tm_mday, 0x1F);
    unsigned8ToPayload(payload, payload_length, 1, tmp.tm_mon, 0x0F);
    unsigned8ToPayload(payload, payload_length, 2, tmp.tm_year % 100, 0x7F);
    return true;
}

int valueToBusValueUnsigned32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(4294967295))
        return false;

    unsigned32ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFFFFFFFF);
    return true;
}

int valueToBusValueSigned32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(-2147483648) || (int64_t)value > INT64_C(2147483647))
        return false;

    signed32ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFFFFFFFF);
    return true;
}

int valueToBusValueLongTimePeriod(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(-2147483648) || (int64_t)value > INT64_C(2147483647))
        return false;

    signed32ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFFFFFFFF);
    return true;
}

int valueToBusValueFloat32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    double numValue = value;
    if (numValue < (-8388608.0 * pow(2, 255)) || numValue > (8388607.0 * pow(2, 255)))
        return false;

    float32ToPayload(payload, payload_length, 0, numValue, 0xFFFFFFFF);
    return true;
}

int valueToBusValueAccess(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(999999))
                return false;
            ENSURE_PAYLOAD(4);
            for (int n = 0, factor = 100000; n < 6; ++n, factor /= 10)
                bcdToPayload(payload, payload_length, n, ((uint64_t)value / factor) % 10);
            break;
        }
        case 1:
        case 2:
        case 3:
        case 4:
            bitToPayload(payload, payload_length, 23 + datatype.index, value);
            break;
        case 5:
        {
            if ((uint64_t)value < INT64_C(0) || (uint64_t)value > INT64_C(15))
                return false;
            bcdToPayload(payload, payload_length, 7, (uint64_t)value);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueString(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    const char* strValue = value;
    uint8_t val = strValue[0];
    for (int n = 0; n < 14; n++) {
        if (val) val = strValue[n]; //string terminator 0x00 will stop further assignments and init the remainig payload with zero
        unsigned8ToPayload(payload, payload_length, n, val, 0xff);
    }
    return true;
}

int valueToBusValueScene(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(63))
        return false;

    unsigned8ToPayload(payload, payload_length, 0, (uint64_t)value, 0xFF);
    return true;
}

int valueToBusValueSceneControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
            bitToPayload(payload, payload_length, 0, value);
            break;
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(63))
                return false;
            unsigned8ToPayload(payload, payload_length, 0, (int64_t)value, 0x3F);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueSceneInfo(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
            bitToPayload(payload, payload_length, 1, value);
            break;
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(63))
                return false;
            unsigned8ToPayload(payload, payload_length, 0, (int64_t)value, 0x3F);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueSceneConfig(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(63))
                return false;
            unsigned8ToPayload(payload, payload_length, 0, (int64_t)value, 0x3F);
            break;
        }
        case 1:
        case 2:
            bitToPayload(payload, payload_length, 2 - datatype.index, value);
            break;
        default:
            return false;
    }

    return true;
}

int valueToBusValueDateTime(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            struct tm local = value;
            time_t time = mktime(&local);

            if (!time) //TODO add check if date or time is invalid
                return false;

            ENSURE_PAYLOAD(8);
            struct tm tmp = value;
            bitToPayload(payload, payload_length, 51, false);
            bitToPayload(payload, payload_length, 52, false);
            unsigned8ToPayload(payload, payload_length, 0, tmp.tm_year - 1900, 0xFF);
            unsigned8ToPayload(payload, payload_length, 1, tmp.tm_mon, 0x0F);
            unsigned8ToPayload(payload, payload_length, 2, tmp.tm_mday, 0x1F);

            bitToPayload(payload, payload_length, 54, false);
            unsigned8ToPayload(payload, payload_length, 3, tmp.tm_hour, 0x1F);
            unsigned8ToPayload(payload, payload_length, 4, tmp.tm_min, 0x3F);
            unsigned8ToPayload(payload, payload_length, 5, tmp.tm_sec, 0x3F);
            break;
        }
        case 1:
        {
            ENSURE_PAYLOAD(8);
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(7))
                bitToPayload(payload, payload_length, 53, true);
            else
            {
                bitToPayload(payload, payload_length, 53, false);
                unsigned8ToPayload(payload, payload_length, 3, (int64_t)value << 5, 0xE0);
            }
            break;
        }
        case 2:
        {
            ENSURE_PAYLOAD(8);
            bitToPayload(payload, payload_length, 49, value);
            bitToPayload(payload, payload_length, 50, false);
            break;
        }
        case 3:
        {
            ENSURE_PAYLOAD(8);
            bitToPayload(payload, payload_length, 48, value);
            break;
        }
        case 9:
        {
            ENSURE_PAYLOAD(8);
            bitToPayload(payload, payload_length, 55, value);
            break;
        }
        case 10:
        {
            bitToPayload(payload, payload_length, 56, value);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueUnicode(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    //TODO
    return false;
}

int valueToBusValueSigned64(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    signed64ToPayload(payload, payload_length, 0, (int64_t)value, UINT64_C(0xFFFFFFFFFFFFFFFF));
    return true;
}

int valueToBusValueAlarmInfo(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(3))
                return false;
            ENSURE_PAYLOAD(6);
            unsigned8ToPayload(payload, payload_length, 1, (int64_t)value, 0xFF);
            break;
        }
        case 0:
        case 2:
        case 3:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(255))
                return false;
            ENSURE_PAYLOAD(6);
            unsigned8ToPayload(payload, payload_length, datatype.index, (int64_t)value, 0xFF);
            break;
        }
        case 4:
        case 5:
        case 6:
        case 7:
        {
            ENSURE_PAYLOAD(6);
            bitToPayload(payload, payload_length, 43 - datatype.index, value);
            break;
        }
        case 8:
        case 9:
        case 10:
        {
            ENSURE_PAYLOAD(6);
            bitToPayload(payload, payload_length, 55 - datatype.index, value);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueSerialNumber(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(65535))
                return false;
            ENSURE_PAYLOAD(6);
            unsigned16ToPayload(payload, payload_length, 0, (int64_t)value, 0xFFFF);
            break;
        }
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(4294967295))
                return false;
            ENSURE_PAYLOAD(6);
            unsigned32ToPayload(payload, payload_length, 2, (int64_t)value, 0xFFFF);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueVersion(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(31))
                return false;
            ENSURE_PAYLOAD(2);
            unsigned8ToPayload(payload, payload_length, 0, (int64_t)value << 3, 0xF8);
            break;
        }
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(31))
                return false;
            unsigned16ToPayload(payload, payload_length, 0, (int64_t)value << 6, 0x07C0);
            break;
        }
        case 2:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(63))
                return false;
            unsigned8ToPayload(payload, payload_length, 1, (int64_t)value, 0x3F);
            break;
        }
        default:
            return false;
    }

    return true;
}

int valueToBusValueScaling(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            uint32_t duration = value;

            if (duration < INT64_C(0) || duration > INT64_C(65535))
                return false;

            ENSURE_PAYLOAD(3);
            unsigned16ToPayload(payload, payload_length, 0, duration, 0xFFFF);
            return true;
        }
        case 1:
        {
            if ((double)value < 0.0 || (double)value > 100.0)
                return false;
            unsigned8ToPayload(payload, payload_length, 2, round((double)value * 255.0 / 100.0), 0xff);
            break;
        }
    }

    return true;
}

int valueToBusValueTariff(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            uint32_t duration = value;

            if (duration < INT64_C(0) || duration > INT64_C(65535))
                return false;

            ENSURE_PAYLOAD(3);
            unsigned16ToPayload(payload, payload_length, 0, duration, 0xFFFF);
            return true;
        }
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(254))
                return false;
            unsigned8ToPayload(payload, payload_length, 2, (int64_t)value, 0xff);
            break;
        }
    }

    return true;
}

int valueToBusValueLocale(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    int strl = strlen(value);
    if (strl != 2)
        return false;

    if (!datatype.index || (datatype.mainGroup == 231 && datatype.index == 1))
    {
        ENSURE_PAYLOAD(datatype.mainGroup == 231 ? 4 : 2);
        unsigned8ToPayload(payload, payload_length, datatype.index * 2, ((const char*)value)[0], 0xff);
        unsigned8ToPayload(payload, payload_length, datatype.index * 2 + 1, ((const char*)value)[1], 0xff);
        return true;
    }

    return false;
}

int valueToBusValueRGB(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(16777215))
        return false;

    unsigned int rgb = (int64_t)value;

    unsigned16ToPayload(payload, payload_length, 0, rgb / 256, 0xffff);
    unsigned8ToPayload(payload, payload_length, 2, rgb % 256, 0xff);
    return true;
}

int valueToBusValueFlaggedScaling(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {
            if ((double)value < 0.0 || (double)value > 100.0)
                return false;
            ENSURE_PAYLOAD(2);
            unsigned8ToPayload(payload, payload_length, 0, round((double)value * 255.0 / 100.0), 0xff);
            break;
        }
        case 1:
            bitToPayload(payload, payload_length, 15, value);
            break;
        default:
            return false;
    }

    return true;
}

int valueToBusValueActiveEnergy(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype)
{
    switch (datatype.index)
    {
        case 0:
        {

            if ((int64_t)value < INT64_C(-2147483648) || (int64_t)value > INT64_C(2147483647))
                return false;
            ENSURE_PAYLOAD(6);
            signed32ToPayload(payload, payload_length, 0, (int64_t)value, 0xffffffff);
            break;
        }
        case 1:
        {
            if ((int64_t)value < INT64_C(0) || (int64_t)value > INT64_C(254))
                return false;
            ENSURE_PAYLOAD(6);
            unsigned8ToPayload(payload, payload_length, 4, (int64_t)value, 0xff);
            break;
        }
        case 2:
        case 3:
            bitToPayload(payload, payload_length, datatype.index + 44, value);
            break;
        default:
            return false;
    }

    return true;
}

// Helper functions
bool bitFromPayload(const uint8_t* payload, int index)
{
    int bit = payload[index / 8] & (1 << (7 - (index % 8)));
    return bit ? true : false;
}
uint8_t unsigned8FromPayload(const uint8_t* payload, int index)
{
    return (uint8_t)payload[index];
}
int8_t signed8FromPayload(const uint8_t* payload, int index)
{
    return (int8_t)payload[index];
}
uint16_t unsigned16FromPayload(const uint8_t* payload, int index)
{
    return ((((uint16_t)payload[index]) << 8) & 0xFF00) | (((uint16_t)payload[index + 1]) & 0x00FF);
}
int16_t signed16FromPayload(const uint8_t* payload, int index)
{
    return ((((uint16_t)payload[index]) << 8) & 0xFF00) | (((uint16_t)payload[index + 1]) & 0x00FF);
}
uint32_t unsigned32FromPayload(const uint8_t* payload, int index)
{
    return ((((uint32_t)payload[index]) << 24) & 0xFF000000) |
           ((((uint32_t)payload[index + 1]) << 16) & 0x00FF0000) |
           ((((uint32_t)payload[index + 2]) << 8) & 0x0000FF00) |
           (((uint32_t)payload[index + 3]) & 0x000000FF);
}
int32_t signed32FromPayload(const uint8_t* payload, int index)
{
    return (int32_t)unsigned32FromPayload(payload, index);
}
uint64_t unsigned64FromPayload(const uint8_t* payload, int index)
{
	return ((((uint64_t)payload[index]) << 56) & 0xFF00000000000000) |
		((((uint64_t)payload[index + 1]) << 48) & 0x00FF000000000000) |
		((((uint64_t)payload[index + 2]) << 40) & 0x0000FF0000000000) |
		((((uint64_t)payload[index + 3]) << 32) & 0x000000FF00000000) |
		((((uint64_t)payload[index + 4]) << 24) & 0x00000000FF000000) |
		((((uint64_t)payload[index + 5]) << 16) & 0x0000000000FF0000) |
		((((uint64_t)payload[index + 6]) << 8) & 0x000000000000FF00) |
		(((uint64_t)payload[index + 7]) & 0x00000000000000FF);
}
double float16FromPayload(const uint8_t* payload, int index)
{
    uint16_t mantissa = unsigned16FromPayload(payload, index) & 0x87FF;
    if (mantissa & 0x8000)
        return ((~mantissa & 0x07FF) + 1.0) * -0.01 * (1 << ((payload[index] >> 3) & 0x0F));

    return mantissa * 0.01 * (1 << ((payload[index] >> 3) & 0x0F));
}
float float32FromPayload(const uint8_t* payload, int index)
{
    union { float f; uint32_t i; } area;
    area.i = unsigned32FromPayload(payload, index);
	return area.f;
}
double float64FromPayload(const uint8_t* payload, int index)
{
	union { double f; uint64_t i; } area;
	area.i = unsigned64FromPayload(payload, index);
    return area.f;
}
int64_t signed64FromPayload(const uint8_t* payload, int index)
{
    return ((((uint64_t)payload[index]) << 56) & UINT64_C(0xFF00000000000000)) |
           ((((uint64_t)payload[index + 1]) << 48) & UINT64_C(0x00FF000000000000)) |
           ((((uint64_t)payload[index + 2]) << 40) & UINT64_C(0x0000FF0000000000)) |
           ((((uint64_t)payload[index + 3]) << 32) & UINT64_C(0x000000FF00000000)) |
           ((((uint64_t)payload[index + 4]) << 24) & UINT64_C(0x00000000FF000000)) |
           ((((uint64_t)payload[index + 5]) << 16) & UINT64_C(0x0000000000FF0000)) |
           ((((uint64_t)payload[index + 6]) << 8) & UINT64_C(0x000000000000FF00)) |
           (((uint64_t)payload[index + 7]) & UINT64_C(0x00000000000000FF));
}
uint8_t bcdFromPayload(const uint8_t* payload, int index)
{
    if (index % 2)
        return (uint8_t)(payload[index / 2] & 0x0F);
    return (uint8_t)((payload[index / 2] >> 4) & 0x0F);
}

void bitToPayload(uint8_t* payload, size_t payload_length, int index, bool value)
{
    ENSURE_PAYLOAD(index / 8 + 1);
    payload[index / 8] = (payload[index / 8] & ~(1 << (7 - (index % 8)))) | (value ? (1 << (7 - (index % 8))) : 0);
}
void unsigned8ToPayload(uint8_t* payload, size_t payload_length, int index, uint8_t value, uint8_t mask)
{
    ENSURE_PAYLOAD(index + 1);
    payload[index] = (payload[index] & ~mask) | (value & mask);
}
void signed8ToPayload(uint8_t* payload, size_t payload_length, int index, int8_t value, uint8_t mask)
{
    ENSURE_PAYLOAD(index + 1);
    payload[index] = (payload[index] & ~mask) | (value & mask);
}
void unsigned16ToPayload(uint8_t* payload, size_t payload_length, int index, uint16_t value, uint16_t mask)
{
    ENSURE_PAYLOAD(index + 2);
    payload[index] = (payload[index] & (~mask >> 8)) | ((value >> 8) & (mask >> 8));
    payload[index + 1] = (payload[index + 1] & ~mask) | (value & mask);
}
void signed16ToPayload(uint8_t* payload, size_t payload_length, int index, int16_t value, uint16_t mask)
{
    ENSURE_PAYLOAD(index + 2);
    payload[index] = (payload[index] & (~mask >> 8)) | ((value >> 8) & (mask >> 8));
    payload[index + 1] = (payload[index + 1] & ~mask) | (value & mask);
}
void unsigned32ToPayload(uint8_t* payload, size_t payload_length, int index, uint32_t value, uint32_t mask)
{
    ENSURE_PAYLOAD(index + 4);
    payload[index] = (payload[index] & (~mask >> 24)) | ((value >> 24) & (mask >> 24));
    payload[index + 1] = (payload[index + 1] & (~mask >> 16)) | ((value >> 16) & (mask >> 16));
    payload[index + 2] = (payload[index + 2] & (~mask >> 8)) | ((value >> 8) & (mask >> 8));
    payload[index + 3] = (payload[index + 3] & ~mask) | (value & mask);
}
void signed32ToPayload(uint8_t* payload, size_t payload_length, int index, int32_t value, uint32_t mask)
{
    ENSURE_PAYLOAD(index + 4);
    payload[index] = (payload[index] & (~mask >> 24)) | ((value >> 24) & (mask >> 24));
    payload[index + 1] = (payload[index + 1] & (~mask >> 16)) | ((value >> 16) & (mask >> 16));
    payload[index + 2] = (payload[index + 2] & (~mask >> 8)) | ((value >> 8) & (mask >> 8));
    payload[index + 3] = (payload[index + 3] & ~mask) | (value & mask);
}

void float16ToPayload(uint8_t* payload, size_t payload_length, int index, double value, uint16_t mask)
{
    bool wasNegative = false;
    if (value < 0)
    {
        wasNegative = true;
        value *= -1;
    }

    value *= 100.0;
    unsigned short exponent = 0;
  
    if(value > 2048)
        exponent = ceil(log2(value) - 11.0);
    
    short mantissa = roundf(value / (1 << exponent));

    if (wasNegative)
        mantissa *= -1;

    println(mantissa);
    
    signed16ToPayload(payload, payload_length, index, mantissa, mask);
    unsigned8ToPayload(payload, payload_length, index, exponent << 3, 0x78 & (mask >> 8));
}
void float32ToPayload(uint8_t* payload, size_t payload_length, int index, double value, uint32_t mask)
{
    union { float f; uint32_t i; } num;
    num.f = value;
    unsigned32ToPayload(payload, payload_length, index, num.i, mask);
}
void signed64ToPayload(uint8_t* payload, size_t payload_length, int index, int64_t value, uint64_t mask)
{
    ENSURE_PAYLOAD(index + 8);
    payload[index] = (payload[index] & (~mask >> 56)) | ((value >> 56) & (mask >> 56));
    payload[index + 1] = (payload[index + 1] & (~mask >> 48)) | ((value >> 48) & (mask >> 48));
    payload[index + 2] = (payload[index + 2] & (~mask >> 40)) | ((value >> 40) & (mask >> 40));
    payload[index + 3] = (payload[index + 3] & (~mask >> 32)) | ((value >> 32) & (mask >> 32));
    payload[index + 4] = (payload[index + 4] & (~mask >> 24)) | ((value >> 24) & (mask >> 24));
    payload[index + 5] = (payload[index + 5] & (~mask >> 16)) | ((value >> 16) & (mask >> 16));
    payload[index + 6] = (payload[index + 6] & (~mask >> 8)) | ((value >> 8) & (mask >> 8));
    payload[index + 7] = (payload[index + 7] & ~mask) | (value & mask);
}
void bcdToPayload(uint8_t* payload, size_t payload_length, int index, uint8_t value)
{
    ENSURE_PAYLOAD(index / 2 + 1);
    if (index % 2)
        payload[index / 2] = (payload[index / 2] & 0xF0) | (value & 0x0F);
    else
        payload[index / 2] = (payload[index / 2] & 0x0F) | ((value << 4) & 0xF0);
}
