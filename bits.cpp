#include "bits.h"

uint8_t* popByte(uint8_t& b, uint8_t* data)
{
    b = *data;
    data += 1;
    return data;
}

uint8_t* popWord(uint16_t& w, uint8_t* data)
{
    w = getWord(data);
    data += 2;
    return data;
}

uint8_t* popInt(uint32_t& i, uint8_t* data)
{
    i = getInt(data);
    data += 4;
    return data;
}

uint8_t* popByteArray(uint8_t* dst, uint32_t size, uint8_t* data)
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

uint16_t getWord(uint8_t* data)
{
    return (data[0] << 8) + data[1];;
}

uint32_t getInt(uint8_t * data)
{
    return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];;
}
