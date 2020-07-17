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
#define MASK_VERSION 0x07B0
//#define MASK_VERSION 0x27B0
//#define MASK_VERSION 0x57B0

// Couplers
// 0x091A: IP/TP1
// 0x2920: TP1/RF
//#define MASK_VERSION 0x091A
//#define MASK_VERSION 0x2920

#define USE_RF
#define USE_TP
#define USE_IP
//#define USE_USB
//#define USE_CEMI_SERVER
#ifdef USE_USB
#define USE_CEMI_SERVER
#endif

#define USE_DATASECURE

#endif
