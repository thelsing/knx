#pragma once

#include "knx/bits.h"
#include "knx/config.h"
#include "knx/bau07B0.h"
#include "knx/bau091A.h"
#include "knx/bau27B0.h"
#include "knx/bau2920.h"
#include "knx/bau57B0.h"

#ifdef ARDUINO_ARCH_SAMD
    #include "samd_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_ESP8266)
    #include "esp_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_ESP32)
    #define LED_BUILTIN 13
    #include "esp32_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_STM32)
    #include "stm32_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif __linux__
    #define LED_BUILTIN 0
    #include "linux_platform.h"
#else
    #define LED_BUILTIN 5 // see GPIO_PinConfig gpioPinConfigs[]
    #include "cc1310_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        extern void buttonUp();
    #endif
#endif

typedef uint8_t* (*SaveRestoreCallback)(uint8_t* buffer);
typedef void (*IsrFunctionPtr)();
 
template <class P, class B> class KnxFacade : private SaveRestore
{
  public:
    KnxFacade() : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
    {
        manufacturerId(0xfa);
        _bau.addSaveRestore(this);
    }

    KnxFacade(B& bau) : _bau(bau)
    {
        manufacturerId(0xfa);
        _bau.addSaveRestore(this);
    }

    KnxFacade(IsrFunctionPtr buttonISRFunction) : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
    {
        manufacturerId(0xfa);
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

    /**
     * returns RISING if interrupt is created in a rising signal, FALLING otherwise
     */
    uint32_t buttonPinInterruptOn()
    {
        return _buttonPinInterruptOn;
    }

    /**
     * Sets if the programming button creates a RISING or a FALLING signal. 
     * 
     * Set to RISING for GPIO--BUTTON--VDD or to FALLING for GPIO--BUTTON--GND
     */
    void buttonPinInterruptOn(uint32_t value)
    {
        _buttonPinInterruptOn = value;
    }

    uint32_t buttonPin()
    {
        return _buttonPin;
    }

    void buttonPin(uint32_t value)
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
                println("progmode on");
                digitalWrite(ledPin(), _ledPinActiveOn);
            }
            else
            {
                println("progmode off");
                digitalWrite(ledPin(), HIGH - _ledPinActiveOn);
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
        pinMode(ledPin(), OUTPUT);

        digitalWrite(ledPin(), HIGH - _ledPinActiveOn);

        pinMode(buttonPin(), INPUT_PULLUP);

        if (_progButtonISRFuncPtr)
        {
            // Workaround for https://github.com/arduino/ArduinoCore-samd/issues/587
            #if (ARDUINO_API_VERSION >= 10200)
                attachInterrupt(_buttonPin, _progButtonISRFuncPtr, (PinStatus)_buttonPinInterruptOn);
            #else
                attachInterrupt(_buttonPin, _progButtonISRFuncPtr, _buttonPinInterruptOn);
            #endif
        }

        enabled(true);
    }

    void setButtonISRFunction(IsrFunctionPtr progButtonISRFuncPtr)
    {
        _progButtonISRFuncPtr = progButtonISRFuncPtr;
    }

    void setSaveCallback(SaveRestoreCallback func)
    {
        _saveCallback = func;
    }

    void setRestoreCallback(SaveRestoreCallback func)
    {
        _restoreCallback = func;
    }

    uint8_t* paramData(uint32_t addr)
    {
        if (!_bau.configured())
            return nullptr;

        return _bau.parameters().data(addr);
    }

    uint8_t paramByte(uint32_t addr)
    {
        if (!_bau.configured())
            return 0;

        return _bau.parameters().getByte(addr);
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
        _bau.restartRequest(individualAddress);
    }

  private:
    P* _platformPtr = 0;
    B* _bauPtr = 0;
    B& _bau;
    uint32_t _ledPinActiveOn = LOW;
    uint32_t _ledPin = LED_BUILTIN;
    uint32_t _buttonPinInterruptOn = RISING;
    uint32_t _buttonPin = 0;
    SaveRestoreCallback _saveCallback = 0;
    SaveRestoreCallback _restoreCallback = 0;
    volatile bool _toggleProgMode = false;
    bool _progLedState = false;
    uint16_t _saveSize = 0;
    IsrFunctionPtr _progButtonISRFuncPtr = 0;

    uint8_t* save(uint8_t* buffer)
    {
        if (_saveCallback != 0)
            return _saveCallback(buffer);

        return buffer;
    }

    uint8_t* restore(uint8_t* buffer)
    {
        if (_restoreCallback != 0)
            return _restoreCallback(buffer);

        return buffer;
    }

    uint16_t saveSize()
    {
        return _saveSize;
    }

    void saveSize(uint16_t size)
    {
        _saveSize = size;
    }
};

#ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
    #ifdef ARDUINO_ARCH_SAMD
        // predefined global instance for TP or RF or TP/RF coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<SamdPlatform, Bau07B0> knx;
        #elif MASK_VERSION == 0x27B0
            extern KnxFacade<SamdPlatform, Bau27B0> knx;
        #elif MASK_VERSION == 0x2920
            extern KnxFacade<SamdPlatform, Bau2920> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_SAMD"
        #endif
    #elif defined(ARDUINO_ARCH_ESP8266)
        // predefined global instance for TP or IP or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<EspPlatform, Bau07B0> knx;
        #elif MASK_VERSION == 0x57B0
            extern KnxFacade<EspPlatform, Bau57B0> knx;
        #elif MASK_VERSION == 0x091A
            extern KnxFacade<EspPlatform, Bau091A> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_ESP8266"
        #endif
    #elif defined(ARDUINO_ARCH_ESP32)
        // predefined global instance for TP or IP or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<Esp32Platform, Bau07B0> knx;
        #elif MASK_VERSION == 0x57B0
            extern KnxFacade<Esp32Platform, Bau57B0> knx;
        #elif MASK_VERSION == 0x091A
            extern KnxFacade<Esp32Platform, Bau091A> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_ESP32"
        #endif
    #elif defined(ARDUINO_ARCH_STM32)
        // predefined global instance for TP only
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<Stm32Platform, Bau07B0> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_STM32"
        #endif
    #else // Non-Arduino platforms and Linux platform
        // no predefined global instance
    #endif
#endif // KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
