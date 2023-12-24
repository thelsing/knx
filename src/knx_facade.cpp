#include "knx_facade.h"

#include "knx/bits.h"

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
            static uint32_t lastEvent=0;
            static uint32_t lastPressed=0;

            uint32_t diff = millis() - lastEvent;
            if (diff >= PROG_BTN_PRESS_MIN_MILLIS && diff <= PROG_BTN_PRESS_MAX_MILLIS){
                if (millis() - lastPressed > 200)
                {  
                    knx.toggleProgMode();
                    lastPressed = millis();
                }
            }
            lastEvent = millis();
        }
    #endif

    #ifdef ARDUINO_ARCH_SAMD
        // predefined global instance for TP or RF or TP/RF coupler
        #if MASK_VERSION == 0x07B0
            KnxFacade<SamdPlatform, Bau07B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x27B0
            KnxFacade<SamdPlatform, Bau27B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x2920
            KnxFacade<SamdPlatform, Bau2920> knx(buttonEvent);
        #else
            #error "Mask version not supported on ARDUINO_ARCH_SAMD"
        #endif
    #elif defined(ARDUINO_ARCH_RP2040)
        // predefined global instance for TP or RF or IP or TP/RF coupler or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            KnxFacade<RP2040ArduinoPlatform, Bau07B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x27B0
            KnxFacade<RP2040ArduinoPlatform, Bau27B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x57B0
            KnxFacade<RP2040ArduinoPlatform, Bau57B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x2920
            KnxFacade<RP2040ArduinoPlatform, Bau2920> knx(buttonEvent);
        #elif MASK_VERSION == 0x091A
            KnxFacade<RP2040ArduinoPlatform, Bau091A> knx(buttonEvent);
        #else
            #error "Mask version not supported on ARDUINO_ARCH_RP2040"
        #endif

    #elif defined(ARDUINO_ARCH_ESP8266)
        // predefined global instance for TP or IP or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            KnxFacade<EspPlatform, Bau07B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x57B0
            KnxFacade<EspPlatform, Bau57B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x091A
            KnxFacade<EspPlatform, Bau091A> knx(buttonEvent);
        #else
            #error "Mask version not supported on ARDUINO_ARCH_ESP8266"
        #endif

    #elif defined(ARDUINO_ARCH_ESP32)
        // predefined global instance for TP or IP or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            KnxFacade<Esp32Platform, Bau07B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x57B0
            KnxFacade<Esp32Platform, Bau57B0> knx(buttonEvent);
        #elif MASK_VERSION == 0x091A
            KnxFacade<Esp32Platform, Bau091A> knx(buttonEvent);
        #else
            #error "Mask version not supported on ARDUINO_ARCH_ESP32"
        #endif

    #elif defined(ARDUINO_ARCH_STM32)
        #if MASK_VERSION == 0x07B0
            KnxFacade<Stm32Platform, Bau07B0> knx(buttonEvent);
        #else
            #error "Mask version not supported on ARDUINO_ARCH_STM32"
        #endif
    #else // Non-Arduino platforms and Linux platform
        // no predefined global instance
    #endif

#endif // KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
