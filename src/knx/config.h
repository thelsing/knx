#pragma once

#ifndef NO_KNX_CONFIG

#ifdef ARDUINO_ARCH_SAMD
#define SPI_SS_PIN 10
#define GPIO_GDO2_PIN 9
#define GPIO_GDO0_PIN 7
#else                    // Linux Platform (Raspberry Pi)
#define SPI_SS_PIN 8     // GPIO 8  (SPI_CE0_N) -> WiringPi: 10 -> Pin number on header: 24
#define GPIO_GDO2_PIN 25 // GPIO 25 (GPIO_GEN6) -> WiringPi: 6  -> Pin number on header: 22
#define GPIO_GDO0_PIN 24 // GPIO 24 (GPIO_GEN5) -> WiringPi: 5  -> Pin number on header: 18
#endif

// Normal devices
// TP1: 0x07B0
// RF: 0x27B0
// IP: 0x57B0
//#define MASK_VERSION 0x07B0
//#define MASK_VERSION 0x27B0
//#define MASK_VERSION 0x57B0

// Couplers
// IP/TP1: 0x091A
// TP1/RF: 0x2920
//#define MASK_VERSION 0x091A
//#define MASK_VERSION 0x2920

// Data Linklayer Driver Options
#if MASK_VERSION == 0x07B0
#define USE_TP
#endif

#if MASK_VERSION == 0x27B0
#define USE_RF
#endif

#if MASK_VERSION == 0x57B0
#define USE_IP
#endif

#if MASK_VERSION == 0x091A
#define USE_TP
#define USE_IP
#endif

#if MASK_VERSION == 0x2920
#define USE_TP
#define USE_RF
#endif

// cEMI options
//#define USE_USB
//#define USE_CEMI_SERVER
#if defined(USE_USB) || defined(KNX_TUNNELING)
#define USE_CEMI_SERVER
#endif

// KNX Data Secure Options
// Define via a compiler -D flag if required
// #define USE_DATASECURE

// option to have GroupObjects (KO in German) use 8 bytes mangement information RAM instead of 19 bytes
// see knx-demo-small-go for example
// this option might be also set via compiler flag -DSMALL_GROUPOBJECT if required
//#define SMALL_GROUPOBJECT

// Some defines to reduce footprint
// Do not perform conversion from KNXValue(const char*) to other types, it mainly avoids the expensive strtod
//#define KNX_NO_STRTOx_CONVERSION
// Do not print messages
//#define KNX_NO_PRINT
// Do not use SPI (Arduino variants)
//#define KNX_NO_SPI
// Do not use the default UART (Arduino variants), it must be defined by ArduinoPlatform::knxUart
// (combined with other flags (HWSERIAL_NONE for stm32) - avoid allocation of RX/TX buffers for all serial lines)
//#define KNX_NO_DEFAULT_UART 

#endif

#if !defined(MASK_VERSION)
#error MASK_VERSION must be defined! See config.h for possible values!
#endif

