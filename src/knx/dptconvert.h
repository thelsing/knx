/*
    KNX client library - internals
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>
    Copyright (C) 2014 Patrik Pfaffenbauer <patrik.pfaffenbauer@p3.co.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License,
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any
    restriction coming from the use of this file. (The General Public
    License restrictions do apply in other respects; for example, they
    cover modification of the file, and distribution when not linked into
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <cstdint>
#include <ctime>

class Dpt
{
  public:
    Dpt() {}
    Dpt(short mainGroup, short subGroup, short index) 
    {
        this->mainGroup = mainGroup;
        this->subGroup = subGroup;
        this->index = index;
    }
    unsigned short mainGroup;
    unsigned short subGroup;
    unsigned short index;
    bool operator==(const Dpt& other) const
    {
        return other.mainGroup == mainGroup && other.subGroup == subGroup && other.index == index;
    }

    bool operator!=(const Dpt& other) const
    {
        return !(other == *this);
    }
};

class KNXValue
{
  public:
    KNXValue() {}
    KNXValue(bool value)  { _value.boolValue = value; }
    KNXValue(uint8_t value) { _value.ucharValue = value; }
    KNXValue(uint16_t value) { _value.ushortValue = value; }
    KNXValue(uint32_t value) { _value.uintValue = value; }
    KNXValue(uint64_t value) { _value.ulongValue = value; }
    KNXValue(int8_t value) { _value.charValue = value; }
    KNXValue(int16_t value) { _value.shortValue = value; }
    KNXValue(int32_t value) { _value.intValue = value; }
    KNXValue(int64_t value) { _value.longValue = value; }
    KNXValue(double value) { _value.doubleValue = value; }
    KNXValue(char* value) { _value.stringValue = value; }
    KNXValue(struct tm value) { _value.timeValue = value; }

    operator bool() const { return _value.boolValue; }
    operator uint8_t() const { return _value.ucharValue; }
    operator uint16_t() const { return _value.ushortValue; }
    operator uint32_t() const { return _value.uintValue; }
    operator uint64_t() const { return _value.ulongValue; }
    operator int8_t() const { return _value.charValue; }
    operator int16_t() const { return _value.shortValue; }
    operator int32_t() const { return _value.intValue; }
    operator int64_t() const { return _value.longValue; }
    operator double() const { return _value.doubleValue; }
    operator char*() const { return _value.stringValue; }
    operator struct tm() const { return _value.timeValue; }

    bool boolValue() const { return _value.boolValue; }
    uint8_t ucharValue() const { return _value.ucharValue; }
    uint16_t ushortValue() const { return _value.ushortValue; }
    uint32_t uintValue() const { return _value.uintValue; }
    uint64_t ulongValue() const { return _value.ulongValue; }
    int8_t charValue() const { return _value.charValue; }
    int16_t shortValue() const { return _value.shortValue; }
    int32_t intValue() const { return _value.intValue; }
    int64_t longValue() const { return _value.longValue; }
    double doubleValue() const { return _value.doubleValue; }
    char* stringValue() const { return _value.stringValue; }
    struct tm timeValue() const { return _value.timeValue; }

    void boolValue(bool value) { _value.boolValue = value; }
    void ucharValue(uint8_t value) { _value.ucharValue = value; }
    void ushortValue(uint16_t value) { _value.ushortValue = value; }
    void uintValue(uint32_t value) { _value.uintValue = value; }
    void ulongValue(uint64_t value) { _value.ulongValue = value; }
    void charValue(int8_t value) { _value.charValue = value; }
    void shortValue(int16_t value) { _value.shortValue = value; }
    void intValue(int32_t value) { _value.intValue = value; }
    void longValue(int64_t value) { _value.longValue = value; }
    void doubleValue(double value) { _value.doubleValue = value; }
    void stringValue(char* value) { _value.stringValue = value; }
    void timeValue(struct tm value) { _value.timeValue = value; }
  private:
    union Value
    {
        bool boolValue;
        uint8_t ucharValue;
        uint16_t ushortValue;
        uint32_t uintValue;
        uint64_t ulongValue;
        int8_t charValue;
        int16_t shortValue;
        int32_t intValue;
        int64_t longValue;
        double doubleValue;
        char* stringValue;
        struct tm timeValue;
    };
    Value _value;
};

/**
 * Converts the KNX Payload given by the specific DPT and puts the value in the KNXValue struc
 */
int KNX_Decode_Value(uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);

/**
 * Converts the KNXValue struct to the KNX Payload as the specific DPT
 */
int KNX_Encode_Value(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);

//KNX to internal
int busValueToBinary(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToBinaryControl(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToStepControl(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToCharacter(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToUnsigned8(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSigned8(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToStatusAndMode(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToUnsigned16(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToTimePeriod(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSigned16(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToTimeDelta(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToFloat16(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToTime(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToDate(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToUnsigned32(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSigned32(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToLongTimePeriod(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToFloat32(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToAccess(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToString(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToScene(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSceneControl(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSceneInfo(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSceneConfig(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToDateTime(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToUnicode(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSigned64(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToAlarmInfo(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToSerialNumber(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToVersion(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToScaling(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToTariff(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToLocale(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToRGB(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToFlaggedScaling(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);
int busValueToActiveEnergy(const uint8_t *payload, int payload_length, const Dpt& datatype, KNXValue& value);

//Internal to KNX
int valueToBusValueBinary(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueBinaryControl(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueStepControl(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueCharacter(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueUnsigned8(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSigned8(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueStatusAndMode(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueUnsigned16(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueTimePeriod(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSigned16(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueTimeDelta(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueFloat16(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueTime(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueDate(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueUnsigned32(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSigned32(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueLongTimePeriod(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueFloat32(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueAccess(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueString(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueScene(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSceneControl(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSceneInfo(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSceneConfig(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueDateTime(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueUnicode(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSigned64(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueAlarmInfo(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueSerialNumber(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueVersion(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueScaling(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueTariff(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueLocale(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueRGB(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueFlaggedScaling(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);
int valueToBusValueActiveEnergy(const KNXValue& value, uint8_t *payload, int payload_length, const Dpt& datatype);

//Payload manipulation
bool bitFromPayload(const uint8_t *payload, int index);
uint8_t unsigned8FromPayload(const uint8_t *payload, int index);
int8_t signed8FromPayload(const uint8_t *payload, int index);
uint16_t unsigned16FromPayload(const uint8_t *payload, int index);
int16_t signed16FromPayload(const uint8_t *payload, int index);
uint32_t unsigned32FromPayload(const uint8_t *payload, int index);
int32_t signed32FromPayload(const uint8_t *payload, int index);
double float16FromPayload(const uint8_t *payload, int index);
float float32FromPayload(const uint8_t *payload, int index);
int64_t signed64FromPayload(const uint8_t *payload, int index);
uint8_t bcdFromPayload(const uint8_t *payload, int index);

void bitToPayload(uint8_t *payload, int payload_length, int index, bool value);
void unsigned8ToPayload(uint8_t *payload, int payload_length, int index, uint8_t value, uint8_t mask);    //mask 0xFF
void signed8ToPayload(uint8_t *payload, int payload_length, int index, int8_t value, uint8_t mask);       //mask 0xFF
void unsigned16ToPayload(uint8_t *payload, int payload_length, int index, uint16_t value, uint16_t mask); //mask 0xFFFF
void signed16ToPayload(uint8_t *payload, int payload_length, int index, int16_t value, uint16_t mask);    //mask 0xFFFF
void unsigned32ToPayload(uint8_t *payload, int payload_length, int index, uint32_t value, uint32_t mask); //mask = 0xFFFFFFFF
void signed32ToPayload(uint8_t *payload, int payload_length, int index, int32_t value, uint32_t mask);    //mask  = 0xFFFFFFFF
void float16ToPayload(uint8_t *payload, int payload_length, int index, double value, uint16_t mask);      //mask = 0xFFFF
void float32ToPayload(uint8_t *payload, int payload_length, int index, double value, uint32_t mask);      //mask  = 0xFFFFFFFF
void signed64ToPayload(uint8_t *payload, int payload_length, int index, int64_t value, uint64_t mask);    //mask = UINT64_C(0xFFFFFFFFFFFFFFFF)
void bcdToPayload(uint8_t *payload, int payload_length, int index, uint8_t value);