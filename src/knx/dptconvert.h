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

#include "dpt.h"
#include "knx_value.h"

/**
 * Converts the KNX Payload given by the specific DPT and puts the value in the KNXValue struc
 */
bool KNX_Decode_Value(uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);

/**
 * Converts the KNXValue struct to the KNX Payload as the specific DPT
 */
bool KNX_Encode_Value(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);

//KNX to internal
bool busValueToBinary(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToBinaryControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToStepControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToCharacter(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToUnsigned8(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSigned8(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToStatusAndMode(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToUnsigned16(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToTimePeriod(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSigned16(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToTimeDelta(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToFloat16(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToTime(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToDate(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToUnsigned32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSigned32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToLongTimePeriod(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToFloat32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToAccess(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToString(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToScene(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSceneControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSceneInfo(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSceneConfig(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToDateTime(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToUnicode(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSigned64(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToAlarmInfo(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToSerialNumber(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToVersion(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToScaling(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToTariff(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToLocale(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToRGB(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToRGBW(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToFlaggedScaling(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
bool busValueToActiveEnergy(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);

//Internal to KNX
bool valueToBusValueBinary(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueBinaryControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueStepControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueCharacter(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueUnsigned8(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSigned8(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueStatusAndMode(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueUnsigned16(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueTimePeriod(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSigned16(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueTimeDelta(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueFloat16(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueTime(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueDate(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueUnsigned32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSigned32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueLongTimePeriod(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueFloat32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueAccess(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueString(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueScene(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSceneControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSceneInfo(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSceneConfig(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueDateTime(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueUnicode(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSigned64(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueAlarmInfo(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueSerialNumber(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueVersion(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueScaling(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueTariff(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueLocale(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueRGB(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueRGBW(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueFlaggedScaling(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
bool valueToBusValueActiveEnergy(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);

//Payload manipulation
bool bitFromPayload(const uint8_t* payload, int index);
uint8_t unsigned8FromPayload(const uint8_t* payload, int index);
int8_t signed8FromPayload(const uint8_t* payload, int index);
uint16_t unsigned16FromPayload(const uint8_t* payload, int index);
int16_t signed16FromPayload(const uint8_t* payload, int index);
uint32_t unsigned32FromPayload(const uint8_t* payload, int index);
int32_t signed32FromPayload(const uint8_t* payload, int index);
uint64_t unsigned64FromPayload(const uint8_t* payload, int index);
double float16FromPayload(const uint8_t* payload, int index);
float float32FromPayload(const uint8_t* payload, int index);
double float64FromPayload(const uint8_t* payload, int index);
int64_t signed64FromPayload(const uint8_t* payload, int index);
uint8_t bcdFromPayload(const uint8_t* payload, int index);

void bitToPayload(uint8_t* payload, size_t payload_length, int index, bool value);
void unsigned8ToPayload(uint8_t* payload, size_t payload_length, int index, uint8_t value, uint8_t mask);    //mask 0xFF
void signed8ToPayload(uint8_t* payload, size_t payload_length, int index, int8_t value, uint8_t mask);       //mask 0xFF
void unsigned16ToPayload(uint8_t* payload, size_t payload_length, int index, uint16_t value, uint16_t mask); //mask 0xFFFF
void signed16ToPayload(uint8_t* payload, size_t payload_length, int index, int16_t value, uint16_t mask);    //mask 0xFFFF
void unsigned32ToPayload(uint8_t* payload, size_t payload_length, int index, uint32_t value, uint32_t mask); //mask = 0xFFFFFFFF
void signed32ToPayload(uint8_t* payload, size_t payload_length, int index, int32_t value, uint32_t mask);    //mask  = 0xFFFFFFFF
void float16ToPayload(uint8_t* payload, size_t payload_length, int index, double value, uint16_t mask);      //mask = 0xFFFF
void float32ToPayload(uint8_t* payload, size_t payload_length, int index, double value, uint32_t mask);      //mask  = 0xFFFFFFFF
void signed64ToPayload(uint8_t* payload, size_t payload_length, int index, int64_t value, uint64_t mask);    //mask = UINT64_C(0xFFFFFFFFFFFFFFFF)
void bcdToPayload(uint8_t* payload, size_t payload_length, int index, uint8_t value);
