#include "knx_facade.h"

#include "knx/bits.h"

#ifdef ARDUINO_ARCH_SAMD
KnxFacade<SamdPlatform, Bau07B0> knx;
#define ICACHE_RAM_ATTR
#elif ARDUINO_ARCH_ESP8266
KnxFacade<EspPlatform, Bau57B0> knx;
#elif ARDUINO_ARCH_ESP32
//KnxFacade<Esp32Platform, Bau57B0> knx;
KnxFacade<Esp32Platform, Bau57B0> knx;
#elif __linux__
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
