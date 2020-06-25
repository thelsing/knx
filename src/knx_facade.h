#pragma once

#include "knx/bits.h"
#include "knx/config.h"
// Set default medium type to TP if no external definitions was given
#ifndef MEDIUM_TYPE
#define MEDIUM_TYPE 0
#endif

#ifdef ARDUINO_ARCH_SAMD
    #include "samd_platform.h"
    #include "knx/bau07B0.h"
    #include "knx/bau27B0.h"
#elif ARDUINO_ARCH_ESP8266
   #include "esp_platform.h"
   #include "knx/bau57B0.h"
#elif ARDUINO_ARCH_ESP32
   #define LED_BUILTIN 13
   #include "esp32_platform.h"
   #include "knx/bau07B0.h"
   #include "knx/bau57B0.h"
#elif ARDUINO_ARCH_STM32
   #include "stm32_platform.h"
   #include "knx/bau07B0.h"
#else
   #define LED_BUILTIN 0
   #include "linux_platform.h"
   #include "knx/bau57B0.h"
   #include "knx/bau27B0.h"
#endif

void buttonUp();
typedef uint8_t* (*SaveRestoreCallback)(uint8_t* buffer);

template <class P, class B> class KnxFacade : private SaveRestore
{
    friend void buttonUp();

  public:
    KnxFacade() : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
    {
        manufacturerId(0xfa);
        _bau.addSaveRestore(this);
    }

    virtual ~KnxFacade()
    {
        if (_bauPtr)
            delete _bauPtr;

        if (_platformPtr)
            delete _platformPtr;
    }

    KnxFacade(B& bau) : _bau(bau)
    {
        manufacturerId(0xfa);
        _bau.addSaveRestore(this);
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

    uint16_t induvidualAddress()
    {
        return _bau.deviceObject().induvidualAddress();
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
        if (_toogleProgMode)
        {
            progMode(!progMode());
            _toogleProgMode = false;
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

    void orderNumber(const char* value)
    {
        _bau.deviceObject().orderNumber(value);
    }

    void hardwareType(uint8_t* value)
    {
        _bau.deviceObject().hardwareType(value);
    }

    void version(uint16_t value)
    {
        _bau.deviceObject().version(value);
    }

    void start()
    {
        pinMode(_ledPin, OUTPUT);

        digitalWrite(_ledPin, HIGH - _ledPinActiveOn);

        pinMode(_buttonPin, INPUT_PULLUP);

        attachInterrupt(_buttonPin, buttonUp, _buttonPinInterruptOn);
        enabled(true);
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

    GroupObject& getGroupObject(uint16_t goNr)
    {
        return _bau.groupObjectTable().get(goNr);
    }

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
    bool _toogleProgMode = false;
    bool _progLedState = false;
    uint16_t _saveSize = 0;

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

#ifdef ARDUINO_ARCH_SAMD
    // predefined global instance for TP or RF
    #ifdef MEDIUM_TYPE
        #if MEDIUM_TYPE == 0
            extern KnxFacade<SamdPlatform, Bau07B0> knx;
        #elif MEDIUM_TYPE == 2
            extern KnxFacade<SamdPlatform, Bau27B0> knx;
        #else
            #error "Only TP and RF supported for Arduino SAMD platform!"
        #endif
    #else
        #error "No medium type specified for Arduino_SAMD platform! Please set MEDIUM_TYPE! (TP:0, RF:2, IP:5)"
    #endif
#elif ARDUINO_ARCH_ESP8266
    // predefined global instance for IP only
    extern KnxFacade<EspPlatform, Bau57B0> knx;
#elif ARDUINO_ARCH_ESP32
    // predefined global instance for TP or IP
    #ifdef MEDIUM_TYPE
        #if MEDIUM_TYPE == 0
            extern KnxFacade<Esp32Platform, Bau07B0> knx;
        #elif MEDIUM_TYPE == 5
            extern KnxFacade<Esp32Platform, Bau57B0> knx;
        #else
            #error "Only TP and IP supported for Arduino ESP32 platform!"
        #endif
    #else
        #error "No medium type specified for Arduino ESP32 platform! Please set MEDIUM_TYPE! (TP:0, RF:2, IP:5)"
    #endif
#elif ARDUINO_ARCH_STM32
    // predefined global instance for TP only
    extern KnxFacade<Stm32Platform, Bau07B0> knx;
#elif __linux__
    // no predefined global instance
#endif