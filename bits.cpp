#include "bits.h"

uint8_t* popByte(uint8_t& b, uint8_t* data)
{
    b = *data;
    data += 1;
    return data;
}

void printHex(const char* suffix, const uint8_t *data, size_t length)
{
    _print(suffix);
    for (int i = 0; i < length; i++) {
        if (data[i] < 0x10) { _print("0"); }
        _print(data[i], HEX);
        _print(" ");
    }
    _println();
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
    return (data[0] << 8) + data[1];
}

uint32_t getInt(uint8_t * data)
{
    return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
}


#ifdef __linux__

void print(const char* s)
{
    printf("%s", s);
}
void print(char c)
{
    printf("%c", c);
}

void print(unsigned char num)
{
    print(num, DEC);
}

void print(unsigned char num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(int num)
{
    print(num, DEC);
}

void print(int num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(unsigned int num)
{
    print(num, DEC);
}

void print(unsigned int num, int base)
{
    if (base == HEX)
        printf("%X", num);
    else
        printf("%d", num);
}

void print(long num)
{
    print(num, DEC);
}

void print(long num, int base)
{
    if (base == HEX)
        printf("%lX", num);
    else
        printf("%ld", num);
}

void print(unsigned long num)
{
    print(num, DEC);
}

void print(unsigned long num, int base)
{
    if (base == HEX)
        printf("%lX", num);
    else
        printf("%ld", num);
}

void print(double num)
{
    printf("%f", num);
}

void println(const char* s)
{
    printf("%s\n", s);
}
void println(char c)
{
    printf("%c\n", c);
}

void println(unsigned char num)
{
    println(num, DEC);
}

void println(unsigned char num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(int num)
{
    println(num, DEC);
}

void println(int num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(unsigned int num)
{
    println(num, DEC);
}

void println(unsigned int num, int base)
{
    if (base == HEX)
        printf("%X\n", num);
    else
        printf("%d\n", num);
}

void println(long num)
{
    println(num, DEC);
}

void println(long num, int base)
{
    if (base == HEX)
        printf("%lX\n", num);
    else
        printf("%ld\n", num);
}

void println(unsigned long num)
{
    println(num, DEC);
}

void println(unsigned long num, int base)
{
    if (base == HEX)
        printf("%lX\n", num);
    else
        printf("%ld\n", num);
}

void println(double num)
{
    printf("%f\n", num);
}

void println(void)
{
    printf("\n");
}

#endif

void printHex(const char* suffix, const uint8_t *data, size_t length)
{
    _print(suffix);
    for (size_t i = 0; i < length; i++) {
        if (data[i] < 0x10) { _print("0"); }
        _print(data[i], HEX);
        _print(" ");
    }
    _print("\n");
}