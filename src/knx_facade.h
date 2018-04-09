#pragma once
#include <Ticker.h>
#include "esp_platform.h"
#include "knx/bau57B0.h"

class RunningState;

class KnxFacade
{
public:
    KnxFacade();
    bool enabled();
    void enabled(bool value);
    bool progMode();
    void progMode(bool value);
    bool configured();
    uint32_t ledPin();
    void ledPin(uint32_t value);
    uint32_t buttonPin();
    void buttonPin(uint32_t value);
    void readMemory();
    void loop();
    void knxLoop();
    void registerGroupObjects(GroupObject* groupObjects, uint16_t count);
    void manufacturerId(uint16_t value);
    void bauNumber(uint32_t value);
    void orderNumber(const char* value);
    void hardwareType(uint8_t* value);
    void version(uint16_t value);
    void start();
    uint8_t* paramData(uint32_t addr);
    uint8_t paramByte(uint32_t addr);
    uint16_t paramWord(uint32_t addr);
    uint32_t paramInt(uint32_t addr);
private:
    EspPlatform _platform;
    Bau57B0 _bau;
    uint32_t _ledPin = 16;
    uint32_t _buttonPin = 0;
    Ticker _ticker;
};

extern KnxFacade knx;