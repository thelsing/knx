#include "knx_facade.h"

#include "knx/bits.h"

#ifdef ARDUINO_ARCH_SAMD
    // predefined global instance for TP or RF or TP/RF coupler
    #if MASK_VERSION == 0x07B0
        KnxFacade<SamdPlatform, Bau07B0> knx;
    #elif MASK_VERSION == 0x27B0
        KnxFacade<SamdPlatform, Bau27B0> knx;
    #elif MASK_VERSION == 0x2920
        KnxFacade<SamdPlatform, Bau2920> knx;
    #else
        #error Mask version not supported on ARDUINO_ARCH_SAMD
    #endif

#elif defined(ARDUINO_ARCH_ESP8266)
    // predefined global instance for TP or IP or TP/IP coupler
    #if MASK_VERSION == 0x07B0
        KnxFacade<EspPlatform, Bau07B0> knx;
    #elif MASK_VERSION == 0x57B0
        KnxFacade<EspPlatform, Bau57B0> knx;
    #elif MASK_VERSION == 0x091A
        KnxFacade<EspPlatform, Bau091A> knx;
    #else
        #error Mask version not supported on ARDUINO_ARCH_ESP8266
    #endif

#elif defined(ARDUINO_ARCH_ESP32)
    // predefined global instance for TP or IP or TP/IP coupler
    #if MASK_VERSION == 0x07B0
        KnxFacade<Esp32Platform, Bau07B0> knx;
    #elif MASK_VERSION == 0x57B0
        KnxFacade<Esp32Platform, Bau57B0> knx;
    #elif MASK_VERSION == 0x091A
        KnxFacade<Esp32Platform, Bau091A> knx;
    #else
        #error Mask version not supported on ARDUINO_ARCH_ESP8266
    #endif

#elif defined(ARDUINO_ARCH_STM32)
    #if MASK_VERSION == 0x07B0
        KnxFacade<Stm32Platform, Bau07B0> knx;
    #else
        #error Mask version not supported on ARDUINO_ARCH_STM32
    #endif
#else // Non-Arduino platforms and Linux platform
    // no predefined global instance
#endif

// Only ESP8266 and ESP32 have this define. For all other platforms this is just empty.
#ifndef ICACHE_RAM_ATTR
    #define ICACHE_RAM_ATTR
#endif

#if (defined(ARDUINO_ARCH_STM32) || \
     defined(ARDUINO_ARCH_ESP32) || \
     defined(ARDUINO_ARCH_ESP8266) || \
     defined(ARDUINO_ARCH_SAMD))
ICACHE_RAM_ATTR void buttonUp()
{
    static uint32_t lastpressed=0;
    if (millis() - lastpressed > 200){
        knx._toogleProgMode = true;
        lastpressed = millis();
    }
}
#elif defined(__linux__)
void buttonUp()
{
    // no de-bounce on linux platform, just satisfy the compiler
}
#endif