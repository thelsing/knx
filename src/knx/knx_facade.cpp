#include "knx_facade.h"

#include "bits.h"

#ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE

#if (defined(ARDUINO_ARCH_STM32) || \
        defined(ARDUINO_ARCH_ESP32) || \
        defined(ARDUINO_ARCH_ESP8266) || \
        defined(ARDUINO_ARCH_SAMD) || \
        defined(ARDUINO_ARCH_RP2040))

// Only ESP8266 and ESP32 have this define. For all other platforms this is just empty.
#ifndef ICACHE_RAM_ATTR
    #define ICACHE_RAM_ATTR
#endif

#ifndef PROG_BTN_PRESS_MIN_MILLIS
    #define PROG_BTN_PRESS_MIN_MILLIS 50
#endif

#ifndef PROG_BTN_PRESS_MAX_MILLIS
    #define PROG_BTN_PRESS_MAX_MILLIS 500
#endif


ICACHE_RAM_ATTR void buttonEvent()
{
    static uint32_t lastEvent = 0;
    static uint32_t lastPressed = 0;

    uint32_t diff = millis() - lastEvent;

    if (diff >= PROG_BTN_PRESS_MIN_MILLIS && diff <= PROG_BTN_PRESS_MAX_MILLIS)
    {
        if (millis() - lastPressed > 200)
        {
            knx.toggleProgMode();
            lastPressed = millis();
        }
    }

    lastEvent = millis();
}
#endif

#ifdef __SAMD51__
    // predefined global instance for TP or RF
    #if MASK_VERSION == 0x07B0
        Knx::KnxFacade<Knx::Samd51Platform, Knx::Bau07B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x27B0
        Knx::KnxFacade<Knx::Samd51Platform, Knx::Bau2920> knx(buttonEvent);
    #elif MASK_VERSION == 0x57B0
        Knx::KnxFacade<Knx::Samd51Platform, Knx::Bau57B0> knx(buttonEvent);
    #else
        #error "Mask version not supported on SAMD51"
    #endif
#elif defined(_SAMD21_)
    // predefined global instance for TP or RF or TP/RF coupler
    #if MASK_VERSION == 0x07B0
        Knx::KnxFacade<Knx::Samd21Platform, Knx::Bau07B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x27B0
        Knx::KnxFacade<Knx::Samd21Platform, Knx::Bau27B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x2920
        Knx::KnxFacade<Knx::Samd21Platform, Knx::Bau2920> knx(buttonEvent);
    #else
        #error "Mask version not supported on SAMD21"
    #endif
#elif defined(ARDUINO_ARCH_RP2040)
    // predefined global instance for TP or RF or IP or TP/RF coupler or TP/IP coupler
    #if MASK_VERSION == 0x07B0
        Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau07B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x27B0
        Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau27B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x57B0
        Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau57B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x2920
        Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau2920> knx(buttonEvent);
    #elif MASK_VERSION == 0x091A
        Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau091A> knx(buttonEvent);
    #else
        #error "Mask version not supported on ARDUINO_ARCH_RP2040"
    #endif

#elif defined(ARDUINO_ARCH_ESP8266)
    // predefined global instance for TP or IP or TP/IP coupler
    #if MASK_VERSION == 0x07B0
        Knx::KnxFacade<Knx::Esp8266Platform, Knx::Bau07B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x57B0
        Knx::KnxFacade<Knx::Esp8266Platform, Knx::Bau57B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x091A
        Knx::KnxFacade<Knx::Esp8266Platform, Knx::Bau091A> knx(buttonEvent);
    #else
        #error "Mask version not supported on ARDUINO_ARCH_ESP8266"
    #endif

#elif defined(ARDUINO_ARCH_ESP32)
    // predefined global instance for TP or IP or TP/IP coupler
    #if MASK_VERSION == 0x07B0
        Knx::KnxFacade<Knx::Esp32Platform, Knx::Bau07B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x57B0
        Knx::KnxFacade<Knx::Esp32Platform, Knx::Bau57B0> knx(buttonEvent);
    #elif MASK_VERSION == 0x091A
        Knx::KnxFacade<Knx::Esp32Platform, Knx::Bau091A> knx(buttonEvent);
    #else
        #error "Mask version not supported on ARDUINO_ARCH_ESP32"
    #endif

#elif defined(ARDUINO_ARCH_STM32)
    #if MASK_VERSION == 0x07B0
        Knx::KnxFacade<Knx::Stm32Platform, Knx::Bau07B0> knx(buttonEvent);
    #else
        #error "Mask version not supported on ARDUINO_ARCH_STM32"
    #endif
#else // Non-Arduino platforms and Linux platform
    // no predefined global instance
#endif

#endif // KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
