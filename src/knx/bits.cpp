#include "bits.h"
#include <cstring> // for memcpy()

const uint8_t* popByte(uint8_t& b, const uint8_t* data)
{
    b = *data;
    data += 1;
    return data;
}

void printHex(const char* suffix, const uint8_t *data, size_t length, bool newline)
{
    print(suffix);
    for (size_t i = 0; i < length; i++) {
        if (data[i] < 0x10) { print("0"); }
        print(data[i], HEX);
        print(" ");
    }
    if (newline)
    {
        println();
    }
}

const uint8_t* popWord(uint16_t& w, const uint8_t* data)
{
    w = getWord(data);
    data += 2;
    return data;
}

const uint8_t* popInt(uint32_t& i, const uint8_t* data)
{
    i = getInt(data);
    data += 4;
    return data;
}

const uint8_t* popByteArray(uint8_t* dst, uint32_t size, const uint8_t* data)
{
    for (uint32_t i = 0; i < size; i++)
        dst[i] = data[i];

    data += size;
    return data;
}

uint8_t* pushByte(uint8_t b, uint8_t* data)
{
    data[0] = b;
    data += 1;
    return data;
}

uint8_t* pushWord(uint16_t w, uint8_t* data)
{
    data[0] = ((w >> 8) & 0xff);
    data[1] = (w & 0xff);
    data += 2;
    return data;
}

uint8_t* pushInt(uint32_t i, uint8_t* data)
{
    data[0] = ((i >> 24) & 0xff);
    data[1] = ((i >> 16) & 0xff);
    data[2] = ((i >> 8) & 0xff);
    data[3] = (i & 0xff);
    data += 4;
    return data;
}

uint8_t* pushByteArray(const uint8_t* src, uint32_t size, uint8_t* data)
{
    for (uint32_t i = 0; i < size; i++)
        data[i] = src[i];
    
    data += size;
    return data;
}

uint16_t getWord(const uint8_t* data)
{
    return (data[0] << 8) + data[1];
}

uint32_t getInt(const uint8_t * data)
{
    return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
}

void sixBytesFromUInt64(uint64_t num, uint8_t* toByteArray)
{
    toByteArray[0] = ((num >> 40) & 0xff);
    toByteArray[1] = ((num >> 32) & 0xff);
    toByteArray[2] = ((num >> 24) & 0xff);
    toByteArray[3] = ((num >> 16) & 0xff);
    toByteArray[4] = ((num >> 8) & 0xff);
    toByteArray[5] = (num & 0xff);
}

uint64_t sixBytesToUInt64(uint8_t* data)
{
    uint64_t l = 0;

    for (uint8_t i = 0; i < 6; i++)
    {
        l = (l << 8) + data[i];
    }
    return l;
}

// The CRC of the Memory Control Block Table Property is a CRC16-CCITT with the following
// parameters:
// Width = 16 bit
// Truncated polynomial = 1021h
// Initial value = FFFFh
// Input date is NOT reflected.
// Output CRC is NOT reflected.
// No XOR is performed on the output CRC.
// EXAMPLE The correct CRC16-CCITT of the string ‘123456789’ is E5CCh.

uint16_t crc16Ccitt(uint8_t* input, uint16_t length)
{
        uint32_t polynom = 0x1021;
        uint8_t padded[length+2];

        memcpy(padded, input, length);
        memset(padded+length, 0x00, 2);

        uint32_t result = 0xffff;
        for (uint32_t i = 0; i < 8 * (uint32_t)sizeof(padded); i++) {
            result <<= 1;
            uint32_t nextBit = (padded[i / 8] >> (7 - (i % 8))) & 0x1;
            result |= nextBit;
            if ((result & 0x10000) != 0)
                result ^= polynom;
        }
        return result & 0xffff;
}

uint16_t crc16Dnp(uint8_t* input, uint16_t length)
{
        // CRC-16-DNP
        // generator polynomial = 2^16 + 2^13 + 2^12 + 2^11 + 2^10 + 2^8 + 2^6 + 2^5 + 2^2 + 2^0
        uint32_t pn = 0x13d65; // 1 0011 1101 0110 0101

        // for much data, using a lookup table would be a way faster CRC calculation
        uint32_t crc = 0;
        for (uint32_t i = 0; i < length; i++) {
            uint8_t bite = input[i] & 0xff;
            for (uint8_t b = 8; b --> 0;) {
                bool bit = ((bite >> b) & 1) == 1;
                bool one = (crc >> 15 & 1) == 1;
                crc <<= 1;
                if (one ^ bit)
                    crc ^= pn;
            }
        }
        return (~crc) & 0xffff;
}
