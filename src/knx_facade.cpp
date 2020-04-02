#include "knx_facade.h"

#include "knx/bits.h"

#ifdef ARDUINO_ARCH_SAMD
    // predefined global instance for TP or RF
    #ifdef MEDIUM_TYPE
        #if MEDIUM_TYPE == 0
            KnxFacade<SamdPlatform, Bau07B0> knx;
        #elif MEDIUM_TYPE == 2
            KnxFacade<SamdPlatform, Bau27B0> knx;
        #else
            #error "Only TP and RF supported for Arduino SAMD platform!"
        #endif
    #else
        #error "No medium type specified for platform Arduino_SAMD! Please set MEDIUM_TYPE! (TP:0, RF:2, IP:5)"
    #endif
    #define ICACHE_RAM_ATTR
#elif ARDUINO_ARCH_ESP8266
    // predefined global instance for IP only
    KnxFacade<EspPlatform, Bau57B0> knx;
#elif ARDUINO_ARCH_ESP32
    // predefined global instance for IP only
    KnxFacade<Esp32Platform, Bau57B0> knx;
#elif ARDUINO_ARCH_STM32
    KnxFacade<Stm32Platform, Bau57B0> knx;
#elif __linux__
    // no predefined global instance
    #define ICACHE_RAM_ATTR
#endif

#ifndef __linux__
uint32_t lastpressed=0;
#endif
ICACHE_RAM_ATTR void buttonUp()
{
    #ifndef __linux__
    if (millis() - lastpressed > 200){
        knx._toogleProgMode = true;
        lastpressed = millis();
    }
    #endif
}
