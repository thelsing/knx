#pragma once

#include "esp_platform.h"
#include "knx/bau57B0.h"

class KnxFacade
{
public:
    KnxFacade();
    bool enabled();
    void enabled(bool value);
    bool progMode();
    void progMode(bool value);
    bool configured();
    void readMemory();
    void loop();
    void registerGroupObjects(GroupObject* groupObjects, uint16_t count);
    void manufacturerId(uint16_t value);
    void bauNumber(uint32_t value);
    void orderNumber(const char* value);
    void hardwareType(uint8_t* value);
    void version(uint16_t value);
    uint8_t* paramData(uint32_t addr);
    uint8_t paramByte(uint32_t addr);
    uint16_t paramWord(uint32_t addr);
    uint32_t paramInt(uint32_t addr);
private:
    EspPlatform _platform;
    Bau57B0 _bau;
};

extern KnxFacade knx;