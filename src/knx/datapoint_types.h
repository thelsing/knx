/*
 *  datapoint_types.h - Conversion functions for datapoint types.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#pragma once
#include <stdint.h>

/**
 * An invalid 2 uint8_t float (DPT9/EIS5).
 * To be used for dptToFloat() and dptFromFloat().
 */
#define INVALID_DPT_FLOAT  2147483647U

/**
 * Convert a value from uint32_t to 2 uint8_t float (DPT9/EIS5). The possible range
 * of the values is -67108864 to 67076096.
 *
 * @param value - the value to convert.
 *                Use INVALID_DPT_FLOAT for the DPT9 "invalid data" value.
 * @return The 2 uint8_t float (DPT9/EIS5).
 */
uint16_t dptToFloat(int32_t value);

/**
 * Convert a value from 2 uint8_t float (DPT9/EIS5) to integer.
 *
 * @param dptValue - the 2 uint8_t float (DPT9/EIS5) to convert
 * @return The value as integer, or INVALID_DPT_FLOAT for the
 *         DPT9 "invalid data" value.
 */
int32_t dptFromFloat(uint16_t dptValue);