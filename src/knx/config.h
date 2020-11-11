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
#ifdef USE_USB
#define USE_CEMI_SERVER
#endif

// KNX Data Secure Options
#define USE_DATASECURE

#endif

#if !defined(MASK_VERSION)
#error MASK_VERSION must be defined! See config.h for possible values!
#endif

