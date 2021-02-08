#pragma once

#include <stdint.h>

class FdskCalculator
{
  public:
    int snprintFdsk(char* str, int strSize, uint8_t* serialNumber, uint8_t* key);

  private:
    char* generateFdskString(uint8_t* serialNumber, uint8_t* key);

    int toBase32(uint8_t* in, long length, uint8_t*& out, bool usePadding);
    int fromBase32(uint8_t* in, long length, uint8_t*& out);

    uint8_t crc4Array(uint8_t* data, uint8_t len) {
        uint8_t start = 0;
        for (uint8_t i = 0; i <len; i++)
        {
            start = crc4(start, data[i]);
        }
        return start;
    }

    uint8_t crc4(uint8_t c, uint8_t x) {
        uint8_t low4Bits = x & 0x0F;
        uint8_t high4Bits = x >> 4;
        c = crc4_tab[c ^ high4Bits];
        c = crc4_tab[c ^ low4Bits];

        return c;
    }

    int ceil(float num);

    static const uint8_t crc4_tab[16];
};
