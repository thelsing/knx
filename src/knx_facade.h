#pragma once

#include "knx/bits.h"

#ifdef ARDUINO_ARCH_SAMD
#include "samd_platform.h"
#include "knx/bau07B0.h"
#elif ARDUINO_ARCH_ESP8266
#include "esp_platform.h"
#include "knx/bau57B0.h"
#elif ARDUINO_ARCH_ESP32
#define LED_BUILTIN 13
#include "esp32_platform.h"
#include "knx/bau57B0.h"
#else
#include "linux_platform.h"
#include "knx/bau57B0.h"
#define LED_BUILTIN 0
#endif

void buttonUp();
typedef void (*saveRestoreCallback)(uint8_t* buffer, uint32_t* size);

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
        _bau.enabled(true);
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

    void setSaveCallback(saveRestoreCallback func)
    {
        _saveCallback = func;
    }

    void setRestoreCallback(saveRestoreCallback func)
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
    saveRestoreCallback _saveCallback = 0;
    saveRestoreCallback _restoreCallback = 0;
    uint32_t (*_sizeCallback)() = 0;
    bool _toogleProgMode = false;
    bool _progLedState = false;

    uint32_t size()
    {
        if (_sizeCallback != 0)
            return _sizeCallback();

        return 0;
    }

    void save()
    {

        if (_saveCallback != 0)
        {
            uint8_t* buffer = NULL;
            uint32_t size=0;
            _saveCallback(buffer, &size);
            if(buffer != NULL){
                _platformPtr->freeNVMemory(_ID);
                uint8_t* addr = _platformPtr->allocNVMemory(size+4, _ID);

                //write size
                _platformPtr->pushNVMemoryByte(((uint8_t*)&size)[0], &addr);
                _platformPtr->pushNVMemoryByte(((uint8_t*)&size)[1], &addr);
                _platformPtr->pushNVMemoryByte(((uint8_t*)&size)[2], &addr);
                _platformPtr->pushNVMemoryByte(((uint8_t*)&size)[3], &addr);
                for(uint32_t i=0;i<size;i++){
                    _platformPtr->pushNVMemoryByte(buffer[i], &addr);
                }
                delete[] buffer;
            }
        }
    }


    void restore(uint8_t* startAddr)
    {
        uint32_t size=0;

        //read size
        ((uint8_t*)&size)[0] = _platformPtr->popNVMemoryByte(&startAddr);
        ((uint8_t*)&size)[1] = _platformPtr->popNVMemoryByte(&startAddr);
        ((uint8_t*)&size)[2] = _platformPtr->popNVMemoryByte(&startAddr);
        ((uint8_t*)&size)[3] = _platformPtr->popNVMemoryByte(&startAddr);

        if (_restoreCallback != 0)
            _restoreCallback(startAddr, &size);

    }
};

#ifdef ARDUINO_ARCH_SAMD
extern KnxFacade<SamdPlatform, Bau07B0> knx;
#elif ARDUINO_ARCH_ESP8266
extern KnxFacade<EspPlatform, Bau57B0> knx;
#elif ARDUINO_ARCH_ESP32
extern KnxFacade<Esp32Platform, Bau57B0> knx;
#elif __linux__
// no predefined global instance
#endif
