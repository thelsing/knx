#pragma once

#ifdef ARDUINO_ARCH_SAMD
#include "samd_platform.h"
#include "knx/bau07B0.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "esp_platform.h"
#include "knx/bau57B0.h"
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
	bool ledPinActiveOn();
	/**
     * @brief To adapt the output to hardware.
	 *
	 * @param ledPinActiveOn = "0" or "low"  --> GPIO--LED--RESISTOR--VDD (for example NODE MCU)
	 * @param ledPinActiveOn = "1" or "high" --> GPIO--RESISTOR--LED--GND (for example WeMos D1 R2)
     */
    void ledPinActiveOn(bool value);
    uint32_t ledPin();
    void ledPin(uint32_t value);
    uint32_t buttonPin();
    void buttonPin(uint32_t value);
    void readMemory();
    void writeMemory();
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
private:
    BauSystemB& _bau;
	bool _ledPinActiveOn = 0;
    uint32_t _ledPin = LED_BUILTIN;
    uint32_t _buttonPin = 0;
#ifdef USE_STATES
    Ticker _ticker;
#endif
    saveRestoreCallback _saveCallback = 0;
    saveRestoreCallback _restoreCallback = 0;
    
    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);
};

extern KnxFacade knx;
