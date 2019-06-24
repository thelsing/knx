#pragma once

#ifdef ARDUINO_ARCH_SAMD
#include "samd_platform.h"
#include "knx/bau07B0.h"
#elif ARDUINO_ARCH_ESP8266
#include "esp_platform.h"
#include "knx/bau57B0.h"
#else
#include "linux_platform.h"
#include "knx/bau57B0.h"
#define LED_BUILTIN 0
#define HIGH 1
#define LOW 0
#endif


typedef uint8_t* (*saveRestoreCallback)(uint8_t* buffer);

class KnxFacade : private SaveRestore
{
public:
    KnxFacade(BauSystemB& bau);
    bool enabled();
    void enabled(bool value);
    bool progMode();
    void progMode(bool value);
    bool configured();
    /**
     * returns HIGH if led is active on HIGH, LOW otherwise
     */
    uint32_t ledPinActiveOn();
    /**
     * Sets if the programming led is active on HIGH or LOW. 
     * 
     * Set to HIGH for GPIO--RESISTOR--LED--GND or to LOW for GPIO--LED--RESISTOR--VDD
     */
    void ledPinActiveOn(uint32_t value);
    uint32_t ledPin();
    void ledPin(uint32_t value);
    uint32_t buttonPin();
    void buttonPin(uint32_t value);
    void readMemory();
    void writeMemory();
    uint16_t induvidualAddress();
    void loop();
    void manufacturerId(uint16_t value);
    void bauNumber(uint32_t value);
    void orderNumber(const char* value);
    void hardwareType(uint8_t* value);
    void version(uint16_t value);
    void start();
    void setSaveCallback(saveRestoreCallback func);
    void setRestoreCallback(saveRestoreCallback func);
    uint8_t* paramData(uint32_t addr);
    uint8_t paramByte(uint32_t addr);
    uint16_t paramWord(uint32_t addr);
    uint32_t paramInt(uint32_t addr);
    GroupObject& getGroupObject(uint16_t goNr);
    void restart(uint16_t individualAddress);

  private:
    BauSystemB& _bau;
	uint32_t _ledPinActiveOn = LOW;
    uint32_t _ledPin = LED_BUILTIN;
    uint32_t _buttonPin = 0;
    saveRestoreCallback _saveCallback = 0;
    saveRestoreCallback _restoreCallback = 0;
    
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
};

#ifndef __linux__
extern KnxFacade knx;
#endif