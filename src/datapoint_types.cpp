/*
 *  datapoint_types.h - Conversion functions for datapoint types.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "datapoint_types.h"
#include <stdint.h>

// Sign for a negative DPT9 float value
#define DPT_FLOAT_NEG_SIGN 0x8000


uint16_t dptToFloat(int32_t value)
{
    uint16_t exp = 0;

    if (value < -67108864 || value > 67076096)
        return 0x7fff;

    if (value < 0)
    {
        while (value < -2048)
        {
            value >>= 1;
            ++exp;
        }

        return DPT_FLOAT_NEG_SIGN | (((int32_t) value) & 2047) | (exp << 11);
    }
    else
    {
        while (value > 2047)
        {
            value >>= 1;
            ++exp;
        }

        return value | (exp << 11);
    }
}

int32_t dptFromFloat(uint16_t dptValue)
{
    uint16_t exp = (dptValue >> 11) & 15;
    int32_t value;

    if (dptValue == 0x7fff)
        return INVALID_DPT_FLOAT;

    if (dptValue >= 0x8000)
        value = dptValue | (-1L & ~2047);
    else value = dptValue & 2047;

    for (; exp; --exp)
        value <<= 1;

    return value;
}
