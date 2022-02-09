#include "fdsk.h"

#include <string.h>

// CRC-4 generator polynom: 10011 (x^4+x+1)
const uint8_t FdskCalculator::crc4_tab[16] =
{
    0x0, 0x3, 0x6, 0x5, 0xc, 0xf, 0xa, 0x9,
    0xb, 0x8, 0xd, 0xe, 0x7, 0x4, 0x1, 0x2
};

int FdskCalculator::snprintFdsk(char* str, int strSize, uint8_t* serialNumber, uint8_t* key)
{
    char* tmpStr = generateFdskString(serialNumber, key);
    int written = 0;

    for (int i = 0; i < 36; i++)
    {
        if (((i % 6) == 0) && (i!=0))
        {
            *(str+written++) = '-';
            if (written >= strSize-1)
                break;
        }
        *(str+written++) = tmpStr[i];
        if (written >= strSize-1)
            break;
    }

    *(str+written++) = '\0';

    delete[] tmpStr;

    return written;
}

char* FdskCalculator::generateFdskString(uint8_t* serialNumber, uint8_t* key)
{
    uint8_t buffer[6 + 16 + 1]; // 6 bytes serialnumber + 16 bytes key + 1 byte placeholder for crc-4
    memcpy(&buffer[0], serialNumber, 6);
    memcpy(&buffer[6], key, 16);
    buffer[22] = (crc4Array(buffer, sizeof(buffer)-1)<<4) &0xFF;

    uint8_t* outEncoded = nullptr;
    toBase32(buffer, sizeof(buffer), outEncoded, false);

    return (char*)outEncoded;
}

int FdskCalculator::ceil(float num)
{
    int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

int FdskCalculator::toBase32(uint8_t* in, long length, uint8_t*& out, bool usePadding)
{
  char base32StandardAlphabet[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"};
  char standardPaddingChar = '=';

  int result = 0;
  int count = 0;
  int bufSize = 8;
  int index = 0;
  int size = 0; // size of temporary array
  uint8_t* temp = nullptr;

  if (length < 0 || length > 268435456LL)
  {
    return 0;
  }

  size = 8 * ceil(length / 4.0); // Calculating size of temporary array. Not very precise.
  temp = new uint8_t[size];

  if (length > 0)
  {
    int buffer = in[0];
    int next = 1;
    int bitsLeft = 8;

    while (bitsLeft > 0 || next < length)
    {
      if (bitsLeft < 5)
      {
        if (next < length)
        {
          buffer <<= 8;
          buffer |= in[next] & 0xFF;
          next++;
          bitsLeft += 8;
        }
        else
        {
          int pad = 5 - bitsLeft;
          buffer <<= pad;
          bitsLeft += pad;
        }
      }
      index = 0x1F & (buffer >> (bitsLeft -5));

      bitsLeft -= 5;
      temp[result] = (uint8_t)base32StandardAlphabet[index];
      result++;
    }
  }

  if (usePadding)
  {
    int pads = (result % 8);
    if (pads > 0)
    {
      pads = (8 - pads);
      for (int i = 0; i < pads; i++)
      {
        temp[result] = standardPaddingChar;
        result++;
      }
    }
  }

  out = new uint8_t[result];

  memcpy(out, temp, result);
  delete [] temp;

  return result;
}

int FdskCalculator::fromBase32(uint8_t* in, long length, uint8_t*& out)
{
  int result = 0; // Length of the array of decoded values.
  int buffer = 0;
  int bitsLeft = 0;
  uint8_t* temp = NULL;

  temp = new uint8_t[length]; // Allocating temporary array.

  for (int i = 0; i < length; i++)
  {
    uint8_t ch = in[i];

    // ignoring some characters: ' ', '\t', '\r', '\n', '='
    if (ch == 0xA0 || ch == 0x09 || ch == 0x0A || ch == 0x0D || ch == 0x3D)
        continue;

    // recovering mistyped: '0' -> 'O', '1' -> 'L', '8' -> 'B'
    if (ch == 0x30)
    {
        ch = 0x4F;
    }
    else if (ch == 0x31)
    {
        ch = 0x4C;
    }
    else if (ch == 0x38)
    {
        ch = 0x42;
    }


    // look up one base32 symbols: from 'A' to 'Z' or from 'a' to 'z' or from '2' to '7'
    if ((ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A))
    {
        ch = ((ch & 0x1F) - 1);
    }
    else if (ch >= 0x32 && ch <= 0x37)
    {
        ch -= (0x32 - 26);
    }
    else {
        delete [] temp;
        return 0;
    }

    buffer <<= 5;
    buffer |= ch;
    bitsLeft += 5;
    if (bitsLeft >= 8)
    {
      temp[result] = (unsigned char)((unsigned int)(buffer >> (bitsLeft - 8)) & 0xFF);
      result++;
      bitsLeft -= 8;
    }
  }

  out = new uint8_t[result];
  memcpy(out, temp, result);
  delete [] temp;

  return result;
}

