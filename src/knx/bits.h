#pragma  once

#include <cstddef>
#include <cstdint>

#if defined(__linux__)
#include <arpa/inet.h>
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32) || defined (DeviceFamily_CC13X0)
#define getbyte(x,n) (*(((uint8_t*)&(x))+n))
#define htons(x)  ( (getbyte(x,0)<<8) | getbyte(x,1) ) 
#define htonl(x) ( (getbyte(x,0)<<24) | (getbyte(x,1)<<16) | (getbyte(x,2)<<8) | getbyte(x,3) )
#define ntohs(x) htons(x)
#define ntohl(x) htonl(x)
#endif

#ifndef MIN
#define MIN(a, b) ((a < b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a > b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(x)    ((x > 0) ? (x) : (-x))
#endif

#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32)
#include <Arduino.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <Arduino.h>
#include <user_interface.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <esp_wifi.h>
#else // Non-Arduino platforms
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
#endif

#ifndef KNX_NO_PRINT
void print(const char[]);
void print(char);
void print(unsigned char, int = DEC);
void print(int, int = DEC);
void print(unsigned int, int = DEC);
void print(long, int = DEC);
void print(unsigned long, int = DEC);
void print(long long, int = DEC);
void print(unsigned long long, int = DEC);
void print(double);

void println(const char[]);
void println(char);
void println(unsigned char, int = DEC);
void println(int, int = DEC);
void println(unsigned int, int = DEC);
void println(long, int = DEC);
void println(unsigned long, int = DEC);
void println(long long, int = DEC);
void println(unsigned long long, int = DEC);
void println(double);
void println(void);

void printHex(const char* suffix, const uint8_t *data, size_t length, bool newline = true);
#else
#define print(...)      do {} while(0)
#define println(...)    do {} while(0)
#define printHex(...)   do {} while(0)
#endif

#ifdef KNX_ACTIVITYCALLBACK
#define KNX_ACTIVITYCALLBACK_DIR 0x00
#define KNX_ACTIVITYCALLBACK_DIR_RECV 0x00
#define KNX_ACTIVITYCALLBACK_DIR_SEND 0x01
#define KNX_ACTIVITYCALLBACK_IPUNICAST 0x02
#define KNX_ACTIVITYCALLBACK_NET 0x04
#endif

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

void sixBytesFromUInt64(uint64_t num, uint8_t* toByteArray);
uint64_t sixBytesToUInt64(uint8_t* data);

uint16_t crc16Ccitt(uint8_t* input, uint16_t length);
uint16_t crc16Dnp(uint8_t* input, uint16_t length);

enum ParameterFloatEncodings
{
    Float_Enc_DPT9 = 0,          // 2 Byte. See Chapter 3.7.2 section 3.10 (Datapoint Types 2-Octet Float Value)
    Float_Enc_IEEE754Single = 1, // 4 Byte. C++ float
    Float_Enc_IEEE754Double = 2, // 8 Byte. C++ double
};


#if defined(ARDUINO_ARCH_SAMD)
// temporary undef until framework-arduino-samd > 1.8.9 is released. See https://github.com/arduino/ArduinoCore-samd/pull/399 for a PR should will probably address this
#undef max
#undef min
// end of temporary undef
#endif
