#include "knx_facade.h"

#include "knx/bits.h"

#ifdef ARDUINO_ARCH_SAMD
KnxFacade<SamdPlatform, Bau07B0> knx;
#define ICACHE_RAM_ATTR
#elif ARDUINO_ARCH_ESP8266
KnxFacade<EspPlatform, Bau57B0> knx;
#elif ARDUINO_ARCH_ESP32
KnxFacade<Esp32Platform, Bau57B0> knx;
#elif __linux__
#define ICACHE_RAM_ATTR
#endif

ICACHE_RAM_ATTR  void buttonUp()
{
    #ifndef __linux__
    knx._toogleProgMode = true;
    #endif
}