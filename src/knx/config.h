#pragma once

#ifdef ARDUINO_ARCH_SAMD
#define SPI_SS_PIN 10
#define GPIO_GDO2_PIN 9
#define GPIO_GDO0_PIN 7
#else                    // Linux Platform (Raspberry Pi)
#define SPI_SS_PIN 8     // GPIO 8  (SPI_CE0_N) -> WiringPi: 10 -> Pin number on header: 24
#define GPIO_GDO2_PIN 25 // GPIO 25 (GPIO_GEN6) -> WiringPi: 6  -> Pin number on header: 22
#define GPIO_GDO0_PIN 24 // GPIO 24 (GPIO_GEN5) -> WiringPi: 5  -> Pin number on header: 18
#endif

#define USE_CEMI_SERVER
#define USE_RF
#define USE_TP
#define USE_IP
