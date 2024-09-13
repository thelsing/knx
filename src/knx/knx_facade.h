#pragma once

#include "util/logger.h"
#include "config.h"
#include "bau/bau.h"
#include "tp/bau07B0.h"
#include "rf/bau27B0.h"
#include "ip/bau57B0.h"
#include "coupler/bau091A.h"
#include "coupler/bau2920.h"
#include "knx/bits.h"

#ifndef USERDATA_SAVE_SIZE
    #define USERDATA_SAVE_SIZE 0
#endif

#ifdef __SAMD51__
#include "platform/samd51_platform.h"
#elif defined(_SAMD21_)
#include "platform/samd21_platform.h"
#elif defined(ARDUINO_ARCH_RP2040)
#include "platform/rp2040_arduino_platform.h"
#elif defined(ARDUINO_ARCH_ESP8266)
#include "platform/esp_platform.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "platform/esp32_platform.h"
#elif defined(ARDUINO_ARCH_STM32)
#include "platform/stm32_platform.h"
#elif __linux__
#include "platform/linux_platform.h"
#else
#include "platform/cc1310_platform.h"
#endif

#ifndef KNX_LED
    #ifdef LED_BUILTIN
        #define KNX_LED LED_BUILTIN
    #else
        #define KNX_LED -1
    #endif
#endif

#ifndef KNX_LED_ACTIVE_ON
    #define KNX_LED_ACTIVE_ON 0
#endif

#ifndef KNX_BUTTON
    #define KNX_BUTTON -1
#endif

namespace Knx
{

#ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
    extern void buttonUp();
#endif

    typedef const uint8_t* (*RestoreCallback)(const uint8_t* buffer);
    typedef uint8_t* (*SaveCallback)(uint8_t* buffer);
    typedef void (*IsrFunctionPtr)();
    typedef void (*ProgLedCallback)(bool on);
#ifdef KNX_ACTIVITYCALLBACK
    typedef void (*ActivityCallback)(uint8_t info);
#endif

    template <class P, class B> class KnxFacade : private SaveRestore
    {
        public:
            KnxFacade() : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
            {
                manufacturerId(0xfa);
                bauNumber(platform().uniqueSerialNumber());
                _bau.addSaveRestore(this);
            }

            KnxFacade(B& bau) : _bau(bau)
            {
                _platformPtr = static_cast<P*>(&bau.platform());
                manufacturerId(0xfa);
                bauNumber(platform().uniqueSerialNumber());
                _bau.addSaveRestore(this);
            }

            KnxFacade(IsrFunctionPtr buttonISRFunction) : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
            {
                manufacturerId(0xfa);
                bauNumber(platform().uniqueSerialNumber());
                _bau.addSaveRestore(this);
                setButtonISRFunction(buttonISRFunction);
            }

            virtual ~KnxFacade()
            {
                if (_bauPtr)
                    delete _bauPtr;

                if (_platformPtr)
                    delete _platformPtr;
            }

            P& platform()
            {
                return *_platformPtr;
            }

            B& bau()
            {
                return _bau;
            }

            bool enabled()
            {
                return _bau.enabled();
            }

            void enabled(bool value)
            {
                _bau.enabled(value);
            }

            bool progMode()
            {
                return _bau.deviceObject().progMode();
            }

            void progMode(bool value)
            {
                _bau.deviceObject().progMode(value);
            }

            /**
             * To be called by ISR handling on button press.
             */
            void toggleProgMode()
            {
                _toggleProgMode = true;
            }

            bool configured()
            {
                return _bau.configured();
            }

            /**
             * returns HIGH if led is active on HIGH, LOW otherwise
             */
            uint32_t ledPinActiveOn()
            {
                return _ledPinActiveOn;
            }

            /**
             * Sets if the programming led is active on HIGH or LOW.
             *
             * Set to HIGH for GPIO--RESISTOR--LED--GND or to LOW for GPIO--LED--RESISTOR--VDD
             */
            void ledPinActiveOn(uint32_t value)
            {
                _ledPinActiveOn = value;
            }

            uint32_t ledPin()
            {
                return _ledPin;
            }

            void ledPin(uint32_t value)
            {
                _ledPin = value;
            }

            void progLedCallback(ProgLedCallback value)
            {
                _progLedCallback = value;
            }

            int32_t buttonPin()
            {
                return _buttonPin;
            }

            void buttonPin(int32_t value)
            {
                _buttonPin = value;
            }

            void readMemory()
            {
                _bau.readMemory();
            }

            void writeMemory()
            {
                _bau.writeMemory();
            }

            uint16_t individualAddress()
            {
                return _bau.deviceObject().individualAddress();
            }

            void loop()
            {
                if (progMode() != _progLedState)
                {
                    _progLedState = progMode();

                    if (_progLedState)
                    {
                        Logger::logger("KnxFacade").info("progmode on");
                        progLedOn();
                    }
                    else
                    {
                        Logger::logger("KnxFacade").info("progmode off");
                        progLedOff();
                    }
                }

                if (_toggleProgMode)
                {
                    progMode(!progMode());
                    _toggleProgMode = false;
                }

                _bau.loop();
            }

            void manufacturerId(uint16_t value)
            {
                _bau.deviceObject().manufacturerId(value);
            }

            void bauNumber(uint32_t value)
            {
                _bau.deviceObject().bauNumber(value);
            }

            void orderNumber(const uint8_t* value)
            {
                _bau.deviceObject().orderNumber(value);
            }

            void hardwareType(const uint8_t* value)
            {
                _bau.deviceObject().hardwareType(value);
            }

            void version(uint16_t value)
            {
                _bau.deviceObject().version(value);
            }

            void start()
            {
                if (_progLedCallback == 0 && _ledPin >= 0)
                    pinMode(ledPin(), OUTPUT);

                progLedOff();

                if (_progButtonISRFuncPtr && _buttonPin >= 0)
                {
                    pinMode(buttonPin(), INPUT_PULLUP);
                    // Workaround for https://github.com/arduino/ArduinoCore-samd/issues/587
#if (ARDUINO_API_VERSION >= 10200)
                    attachInterrupt(_buttonPin, _progButtonISRFuncPtr, (PinStatus)CHANGE);
#else
                    attachInterrupt(_buttonPin, _progButtonISRFuncPtr, CHANGE);
#endif
                }

                enabled(true);
            }

            void setButtonISRFunction(IsrFunctionPtr progButtonISRFuncPtr)
            {
                _progButtonISRFuncPtr = progButtonISRFuncPtr;
            }

            void setSaveCallback(SaveCallback func)
            {
                _saveCallback = func;
            }

            void setRestoreCallback(RestoreCallback func)
            {
                _restoreCallback = func;
            }

            uint8_t* paramData(uint32_t addr)
            {
                if (!_bau.configured())
                    return nullptr;

                return _bau.parameters().data(addr);
            }

            // paramBit(address, shift)
            // get state of a parameter as a boolean like "enable/disable", ...
            // Declaration in XML file:
            // ...
            // <ParameterType Id="M-00FA_A-0066-EA-0001_PT-toggle" Name="toggle">
            //   <TypeRestriction Base="Value" SizeInBit="1">
            //     <Enumeration Text="Désactivé" Value="0" Id="M-00FA_A-0066-EA-0001_PT-toggle_EN-0"/>
            //     <Enumeration Text="Activé" Value="1" Id="M-00FA_A-0066-EA-0001_PT-toggle_EN-1"/>
            //  </TypeRestriction>
            // </ParameterType>
            // ...
            // <Parameter Id="M-00FA_A-0066-EA-0001_P-2" Name="Input 1" ParameterType="M-00FA_A-0066-EA-0001_PT-toggle" Text="Input 1" Value="1">
            //   <Memory CodeSegment="M-00FA_A-0066-EA-0001_RS-04-00000" Offset="1" BitOffset="0"/>
            // </Parameter>
            // <Parameter Id="M-00FA_A-0066-EA-0001_P-3" Name="Input 2" ParameterType="M-00FA_A-0066-EA-0001_PT-toggle" Text="Input 2" Value="1">
            //   <Memory CodeSegment="M-00FA_A-0066-EA-0001_RS-04-00000" Offset="1" BitOffset="1"/>
            // </Parameter>
            // <Parameter Id="M-00FA_A-0066-EA-0001_P-4" Name="Inout 3" ParameterType="M-00FA_A-0066-EA-0001_PT-toggle" Text="Input 3" Value="1">
            //   <Memory CodeSegment="M-00FA_A-0066-EA-0001_RS-04-00000" Offset="1" BitOffset="2"/>
            // </Parameter>
            // ...
            // Usage in code :
            //   if ( knx.paramBit(1,1))
            //   {
            //      //do somthings ....
            //   }
            bool paramBit(uint32_t addr, uint8_t shift)
            {
                if (!_bau.configured())
                    return 0;

                return (bool) ((_bau.parameters().getByte(addr) >> (7 - shift)) & 0x01);
            }

            uint8_t paramByte(uint32_t addr)
            {
                if (!_bau.configured())
                    return 0;

                return _bau.parameters().getByte(addr);
            }

            // Same usage than paramByte(addresse) for signed parameters
            // Declaration in XML file
            // <ParameterType Id="M-00FA_A-0066-EA-0001_PT-delta" Name="delta">
            //   <TypeNumber SizeInBit="8" Type="signedInt" minInclusive="-10" maxInclusive="10"/>
            // </ParameterType>
            int8_t paramSignedByte(uint32_t addr)
            {
                if (!_bau.configured())
                    return 0;

                return (int8_t) _bau.parameters().getByte(addr);
            }

            uint16_t paramWord(uint32_t addr)
            {
                if (!_bau.configured())
                    return 0;

                return _bau.parameters().getWord(addr);
            }

            uint32_t paramInt(uint32_t addr)
            {
                if (!_bau.configured())
                    return 0;

                return _bau.parameters().getInt(addr);
            }

            double paramFloat(uint32_t addr, ParameterFloatEncodings enc)
            {
                if (!_bau.configured())
                    return 0;

                return _bau.parameters().getFloat(addr, enc);
            }

#if (MASK_VERSION == 0x07B0) || (MASK_VERSION == 0x27B0) || (MASK_VERSION == 0x57B0)
            GroupObject& getGroupObject(uint16_t goNr)
            {
                return _bau.groupObjectTable().get(goNr);
            }
#endif

            void restart(uint16_t individualAddress)
            {
                SecurityControl sc = {false, None};
                _bau.restartRequest(individualAddress, sc);
            }

            void beforeRestartCallback(BeforeRestartCallback func)
            {
                _bau.beforeRestartCallback(func);
            }

            BeforeRestartCallback beforeRestartCallback()
            {
                return _bau.beforeRestartCallback();
            }

        private:
            P* _platformPtr = 0;
            B* _bauPtr = 0;
            B& _bau;
            ProgLedCallback _progLedCallback = 0;
#ifdef KNX_ACTIVITYCALLBACK
            ActivityCallback _activityCallback = 0;
#endif
            uint32_t _ledPinActiveOn = KNX_LED_ACTIVE_ON;
            int32_t _ledPin = KNX_LED;
            int32_t _buttonPin = KNX_BUTTON;
            SaveCallback _saveCallback = 0;
            RestoreCallback _restoreCallback = 0;
            volatile bool _toggleProgMode = false;
            bool _progLedState = false;
            uint16_t _saveSize = USERDATA_SAVE_SIZE;
            IsrFunctionPtr _progButtonISRFuncPtr = 0;

            uint8_t* save(uint8_t* buffer)
            {
                if (_saveCallback != 0)
                    return _saveCallback(buffer);

                return buffer;
            }

            const uint8_t* restore(const uint8_t* buffer)
            {
                if (_restoreCallback != 0)
                    return _restoreCallback(buffer);

                return buffer;
            }

            const char* name() override
            {
                return "KnxFacade";
            }

            uint16_t saveSize()
            {
                return _saveSize;
            }

            void saveSize(uint16_t size)
            {
                _saveSize = size;
            }

            void progLedOn()
            {
                if (_progLedCallback == 0)
                    digitalWrite(ledPin(), _ledPinActiveOn);
                else
                    _progLedCallback(true);
            }

            void progLedOff()
            {
                if (_progLedCallback == 0)
                    digitalWrite(ledPin(), HIGH - _ledPinActiveOn);
                else
                    _progLedCallback(false);
            }
    };
}
#ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
#ifdef __SAMD51__
// predefined global instance for TP or RF
#if MASK_VERSION == 0x07B0
    extern Knx::KnxFacade<Knx::Samd51Platform, Knx::Bau07B0> knx;
#elif MASK_VERSION == 0x27B0
    extern Knx::KnxFacade<Knx::Samd51Platform, Knx::Bau2920> knx;
#elif MASK_VERSION == 0x57B0
    extern Knx::KnxFacade<Knx::Samd51Platform, Knx::Bau57B0> knx;
#else
    #error "Mask version not supported on SAMD51"
#endif
#elif defined(_SAMD21_)
// predefined global instance for TP or RF or TP/RF coupler
#if MASK_VERSION == 0x07B0
    extern Knx::KnxFacade<Knx::Samd21Platform, Knx::Bau07B0> knx;
#elif MASK_VERSION == 0x27B0
    extern Knx::KnxFacade<Knx::Samd21Platform, Knx::Bau27B0> knx;
#elif MASK_VERSION == 0x2920
    extern Knx::KnxFacade<Knx::Samd21Platform, Knx::Bau2920> knx;
#else
    #error "Mask version not supported on ARDUINO_ARCH_SAMD"
#endif
#elif defined(ARDUINO_ARCH_RP2040)
// predefined global instance for TP or RF or TP/RF or TP/IP coupler
#if MASK_VERSION == 0x07B0
    extern Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau07B0> knx;
#elif MASK_VERSION == 0x27B0
    extern Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau27B0> knx;
#elif MASK_VERSION == 0x57B0
    extern Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau57B0> knx;
#elif MASK_VERSION == 0x2920
    extern Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau2920> knx;
#elif MASK_VERSION == 0x091A
    extern Knx::KnxFacade<Knx::RP2040ArduinoPlatform, Knx::Bau091A> knx;
#else
    #error "Mask version not supported on ARDUINO_ARCH_RP2040"
#endif
#elif defined(ARDUINO_ARCH_ESP8266)
// predefined global instance for TP or IP or TP/IP coupler
#if MASK_VERSION == 0x07B0
    extern Knx::KnxFacade<Knx::EspPlatform, Knx::Bau07B0> knx;
#elif MASK_VERSION == 0x57B0
    extern Knx::KnxFacade<Knx::EspPlatform, Knx::Bau57B0> knx;
#elif MASK_VERSION == 0x091A
    extern Knx::KnxFacade<Knx::EspPlatform, Knx::Bau091A> knx;
#else
    #error "Mask version not supported on ARDUINO_ARCH_ESP8266"
#endif
#elif defined(ARDUINO_ARCH_ESP32)
// predefined global instance for TP or IP or TP/IP coupler
#if MASK_VERSION == 0x07B0
    extern Knx::KnxFacade<Knx::Esp32Platform, Knx::Bau07B0> knx;
#elif MASK_VERSION == 0x57B0
    extern Knx::KnxFacade<Knx::Esp32Platform, Knx::Bau57B0> knx;
#elif MASK_VERSION == 0x091A
    extern Knx::KnxFacade<Knx::Esp32Platform, Knx::Bau091A> knx;
#else
    #error "Mask version not supported on ARDUINO_ARCH_ESP32"
#endif
#elif defined(ARDUINO_ARCH_STM32)
// predefined global instance for TP only
#if MASK_VERSION == 0x07B0
    extern Knx::KnxFacade<Knx::Stm32Platform, Knx::Bau07B0> knx;
#else
    #error "Mask version not supported on ARDUINO_ARCH_STM32"
#endif
#else // Non-Arduino platforms and Linux platform
// no predefined global instance
#endif
#endif // KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
