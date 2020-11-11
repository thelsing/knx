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

#define INPUT (0x0)
#define OUTPUT (0x1)
#define INPUT_PULLUP (0x2)
#define INPUT_PULLDOWN (0x3)

#define LOW (0x0)
#define HIGH (0x1)
#define CHANGE 2
#define FALLING 3
#define RISING 4

void delay(uint32_t millis);
void delayMicroseconds (unsigned int howLong);
uint32_t millis();
void pinMode(uint32_t dwPin, uint32_t dwMode);
void digitalWrite(uint32_t dwPin, uint32_t dwVal);
uint32_t digitalRead(uint32_t dwPin);
typedef void (*voidFuncPtr)(void);
void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode);

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32)
#include <Arduino.h>

#define getbyte(x,n) (*(((uint8_t*)&(x))+n))
#define htons(x)  ( (getbyte(x,0)<<8) | getbyte(x,1) ) 
#define htonl(x) ( (getbyte(x,0)<<24) | (getbyte(x,1)<<16) | (getbyte(x,2)<<8) | getbyte(x,3) )
#define ntohs(x) htons(x)
#define ntohl(x) htonl(x)

#elif defined(ARDUINO_ARCH_ESP8266)
#include <Arduino.h>
#include <user_interface.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <esp_wifi.h>
#endif

void print(const char[]);
void print(char);
void print(unsigned char, int = DEC);
void print(int, int = DEC);
void print(unsigned int, int = DEC);
void print(long, int = DEC);
void print(unsigned long, int = DEC);
void print(unsigned long long, int = DEC);
void print(double);

void println(const char[]);
void println(char);
void println(unsigned char, int = DEC);
void println(int, int = DEC);
void println(unsigned int, int = DEC);
void println(long, int = DEC);
void println(unsigned long, int = DEC);
void println(unsigned long long, int = DEC);
void println(double);
void println(void);



void printHex(const char* suffix, const uint8_t *data, size_t length);

const uint8_t* popByte(uint8_t& b, const uint8_t* data);
const uint8_t* popWord(uint16_t& w, const uint8_t* data);
const uint8_t* popInt(uint32_t& i, const uint8_t* data);
const uint8_t* popByteArray(uint8_t* dst, uint32_t size, const uint8_t* data);
uint8_t* pushByte(uint8_t b, uint8_t* data);
uint8_t* pushWord(uint16_t w, uint8_t* data);
uint8_t* pushInt(uint32_t i, uint8_t* data);
uint8_t* pushByteArray(const uint8_t* src, uint32_t size, uint8_t* data);
uint16_t getWord(const uint8_t* data);
uint32_t getInt(const uint8_t* data);
void printHex(const char* suffix, const uint8_t *data, size_t length);

void sixBytesFromUInt64(uint64_t num, uint8_t* toByteArray);
uint64_t sixBytesToUInt64(uint8_t* data);

uint16_t crc16Ccitt(uint8_t* input, uint16_t length);
