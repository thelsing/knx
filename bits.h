/*
 *  bits.h - Bit and uint8_t manipulation functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#pragma  once
#include <stdint.h>
/**
 * Compute the value of the specified bit.
 *
 * @param bitno - the number of the bit (0, 1, 2... 31)
 */
#define bit(bitno) (1UL << (bitno))

/**
 * Clear the bit of a number. The number can be
 * any integer (uint8_t, short, uint32_t, long).
 *
 * @param val - the number from which to clear the bit
 * @param bitno - the number of the bit (0, 1, 2... 31)
 */
#define bitClear(val, bitno) ((val) &= ~(1UL << (bitno)))

/**
 * Set the bit of a number. The number can be
 * any integer (uint8_t, short, uint32_t, long).
 *
 * @param val - the number from which to set the bit
 * @param bitno - the number of the bit (0, 1, 2... 31)
 */
#define bitSet(val, bitno) ((val) |= 1UL << (bitno))

/**
 * Write the value of a bit of a number.
 *
 * @param val - the number from which to write the bit
 * @param bitno - the number of the bit (0, 1, 2... 31)
 * @param b - the bit value (0 or 1)
 */
#define bitWrite(val, bitno, b) ((b) ? bitSet(val, bitno) : bitClear(val, bitno))

/**
 * Read the value of a bit of a number. The number can be
 * any integer (uint8_t, short, uint32_t, long).
 *
 * @param val - the number from which to get the bit
 * @param bitno - the number of the bit (0, 1, 2... 31)
 * @return The value of the bit (0 or 1).
 */
#define bitRead(val, bitno) (((val) >> (bitno)) & 1)

/**
 * Extract the lowest (rightmost) uint8_t of a number. The number can be
 * any integer (uint8_t, short, uint32_t, long).
 *
 * @param val - the value to extract the lowest uint8_t.
 * @return The extracted uint8_t (0..255)
 */
#define lowByte(val) ((val) & 255)

/**
 * Extract the highest (leftmost) uint8_t of a number. The number can be
 * any integer (uint8_t, short, uint32_t, long).
 *
 * @param val - the value to extract the highest uint8_t.
 * @return The extracted uint8_t (0..255)
 */
#define highByte(val) (((val) >> ((sizeof(val) - 1) << 3)) & 255)

/**
 * Combine two bytes to a 16 bit uint16_t.
 *
 * @param high - the high uint8_t.
 * @param low - the low uint8_t.
 * @return The bytes combined as uint16_t.
 */
uint16_t makeWord(uint8_t high, uint8_t low);

/**
 * Reverse the uint8_t order of an integer.
 *
 * @param val - the value to reverse.
 * @return The value with reversed uint8_t order.
 */
 uint32_t reverseByteOrder(uint32_t val);

/**
 * Reverse the uint8_t order of a short integer.
 *
 * @param val - the value to reverse.
 * @return The value with reversed uint8_t order.
 */
 uint16_t reverseByteOrder(uint16_t val);


//
//  Inline functions
//

inline uint16_t makeWord(uint8_t high, uint8_t low)
{
    return (high << 8) | low;
}

inline uint32_t reverseByteOrder(uint32_t val)
{
	 uint32_t swapped = ((val >> 24) & 0xff) | // move uint8_t 3 to uint8_t 0
		((val << 8) & 0xff0000) | // move uint8_t 1 to uint8_t 2
		((val >> 8) & 0xff00) | // move uint8_t 2 to uint8_t 1
		((val << 24) & 0xff000000); // uint8_t 0 to uint8_t 3
	return swapped;//__REV(val);
}

inline uint16_t reverseByteOrder(uint16_t val)
{
    uint16_t swapped = (val >> 8) | (val << 8);
	return swapped;
}

uint8_t* popByte(uint8_t& b, uint8_t* data);
uint8_t* popWord(uint16_t& w, uint8_t* data);
uint8_t* popInt(uint32_t& i, uint8_t* data);
uint8_t* popByteArray(uint8_t* dst, uint32_t size, uint8_t* data);
uint8_t* pushByte(uint8_t b, uint8_t* data);
uint8_t* pushWord(uint16_t w, uint8_t* data);
uint8_t* pushInt(uint32_t i, uint8_t* data);
uint8_t* pushByteArray(const uint8_t* src, uint32_t size, uint8_t* data);
uint16_t getWord(uint8_t* data);
uint32_t getInt(uint8_t* data);