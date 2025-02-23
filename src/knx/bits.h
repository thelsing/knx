#pragma once

#include <cstddef>
#include <stdint.h>

#if defined(__linux__)
#include <arpa/inet.h>
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32) || defined(DeviceFamily_CC13X0)
#define htons(x) (((x) << 8 & 0xFF00) | \
                  ((x) >> 8 & 0x00FF))
#define ntohs(x) htons(x)

#define htonl(x) (((x) << 24 & 0xFF000000UL) | \
                  ((x) << 8 & 0x00FF0000UL) |  \
                  ((x) >> 8 & 0x0000FF00UL) |  \
                  ((x) >> 24 & 0x000000FFUL))
#define ntohl(x) htonl(x)
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
#define ABS(x) ((x > 0) ? (x) : (-x))
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
#define lowByte(val) ((val) & 255)
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
void delayMicroseconds(unsigned int howLong);
uint32_t millis();
void pinMode(uint32_t dwPin, uint32_t dwMode);
void digitalWrite(uint32_t dwPin, uint32_t dwVal);
uint32_t digitalRead(uint32_t dwPin);
typedef void (*voidFuncPtr)(void);
void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode);
#endif
namespace Knx
{
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

    void printHex(const char* suffix, const uint8_t* data, size_t length, bool newline = true);
#else
#define print(...) \
    do             \
    {              \
    } while (0)
#define println(...) \
    do               \
    {                \
    } while (0)
#define printHex(...) \
    do                \
    {                 \
    } while (0)
#define byte2hex(...) ""
#define word2hex(...) ""
#define array2hex(...) ""
#define enum_name(...) ""
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

#if defined(ARDUINO_ARCH_SAMD)
// temporary undef until framework-arduino-samd > 1.8.9 is released. See https://github.com/arduino/ArduinoCore-samd/pull/399 for a PR should will probably address this
#undef max
#undef min
    // end of temporary undef
#endif
} // namespace Knx