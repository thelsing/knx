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

#include "../knx_value.h"
#include "dpt.h"

#include <cstdint>

namespace Knx
{
    /**
     * Converts the KNX Payload given by the specific DPT and puts the value in the KNXValue struc
     */
    int KNX_Decode_Value(uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);

    /**
     * Converts the KNXValue struct to the KNX Payload as the specific DPT
     */
    int KNX_Encode_Value(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);

    // KNX to internal
    int busValueToUnsigned32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToSigned32(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToSceneControl(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToSceneInfo(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToSceneConfig(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToDateTime(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToUnicode(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToSigned64(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToAlarmInfo(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToSerialNumber(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToVersion(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToScaling(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToTariff(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToLocale(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToRGB(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToRGBW(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToFlaggedScaling(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);
    int busValueToActiveEnergy(const uint8_t* payload, size_t payload_length, const Dpt& datatype, KNXValue& value);

    // Internal to KNX
    int valueToBusValueUnsigned32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueSigned32(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueSceneControl(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueSceneInfo(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueSceneConfig(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueDateTime(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueUnicode(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueSigned64(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueAlarmInfo(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueSerialNumber(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueVersion(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueScaling(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueTariff(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueLocale(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueRGB(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueRGBW(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueFlaggedScaling(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);
    int valueToBusValueActiveEnergy(const KNXValue& value, uint8_t* payload, size_t payload_length, const Dpt& datatype);

    // Payload manipulation
    bool bitFromPayload(const uint8_t* payload, int index);
    uint8_t unsigned8FromPayload(const uint8_t* payload, int index);
    int8_t signed8FromPayload(const uint8_t* payload, int index);
    uint16_t unsigned16FromPayload(const uint8_t* payload, int index);
    int16_t signed16FromPayload(const uint8_t* payload, int index);
    uint32_t unsigned32FromPayload(const uint8_t* payload, int index);
    int32_t signed32FromPayload(const uint8_t* payload, int index);
    uint64_t unsigned64FromPayload(const uint8_t* payload, int index);
    float float16FromPayload(const uint8_t* payload, int index);
    float float32FromPayload(const uint8_t* payload, int index);
    double float64FromPayload(const uint8_t* payload, int index);
    int64_t signed64FromPayload(const uint8_t* payload, int index);
    uint8_t bcdFromPayload(const uint8_t* payload, int index);

    void bitToPayload(uint8_t* payload, int index, bool value);
    void unsigned8ToPayload(uint8_t* payload, int index, uint8_t value, uint8_t mask);    // mask 0xFF
    void signed8ToPayload(uint8_t* payload, int index, int8_t value, uint8_t mask);       // mask 0xFF
    void unsigned16ToPayload(uint8_t* payload, int index, uint16_t value, uint16_t mask); // mask 0xFFFF
    void signed16ToPayload(uint8_t* payload, int index, int16_t value, uint16_t mask);    // mask 0xFFFF
    void unsigned32ToPayload(uint8_t* payload, int index, uint32_t value, uint32_t mask); // mask = 0xFFFFFFFF
    void signed32ToPayload(uint8_t* payload, int index, int32_t value, uint32_t mask);    // mask  = 0xFFFFFFFF
    void float16ToPayload(uint8_t* payload, int index, float value, uint16_t mask);       // mask = 0xFFFF
    void float32ToPayload(uint8_t* payload, int index, float value, uint32_t mask);       // mask  = 0xFFFFFFFF
    void signed64ToPayload(uint8_t* payload, int index, int64_t value, uint64_t mask);    // mask = UINT64_C(0xFFFFFFFFFFFFFFFF)
    void bcdToPayload(uint8_t* payload, int index, uint8_t value);
} // namespace Knx