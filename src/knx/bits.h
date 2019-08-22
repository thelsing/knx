#pragma  once

#include <cstddef>
#include <cstdint>

#ifdef __linux__
#include <arpa/inet.h>

#define lowByte(val) ((val)&255)
#define highByte(val) (((val) >> ((sizeof(val) - 1) << 3)) & 255)
#define bitRead(val, bitno) (((val) >> (bitno)) & 1)

// print functions are implemented in the platform files
#define DEC 10
#define HEX 16

void delay(uint32_t millis);
uint32_t millis();

#elif ARDUINO_ARCH_SAMD
#include <Arduino.h>
#define htons(x) ( (((x)<<8)&0xFF00) | (((x)>>8)&0xFF) )
#define ntohs(x) htons(x)
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)
#elif ARDUINO_ARCH_ESP8266
#include <Arduino.h>
#include <user_interface.h>
#endif

void print(const char[]);
void print(char);
void print(unsigned char, int = DEC);
void print(int, int = DEC);
void print(unsigned int, int = DEC);
void print(long, int = DEC);
void print(unsigned long, int = DEC);
void print(double, int = 2);

void println(const char[]);
void println(char);
void println(unsigned char, int = DEC);
void println(int, int = DEC);
void println(unsigned int, int = DEC);
void println(long, int = DEC);
void println(unsigned long, int = DEC);
void println(double, int = 2);
void println(void);



void printHex(const char* suffix, const uint8_t *data, size_t length);

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
void printHex(const char* suffix, const uint8_t *data, size_t length);