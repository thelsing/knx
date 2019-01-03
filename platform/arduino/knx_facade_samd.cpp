#include "knx_facade.h"
#if 0
KnxFacade knx;


#define SerialDBG SerialUSB

void buttonUp();
long buttonTimestamp = 0;
void buttonDown()
{
    buttonTimestamp = millis();
    attachInterrupt(knx.buttonPin(), buttonUp, RISING);
}

void buttonUp()
{
    // keep short/long for now 
    if (millis() - buttonTimestamp > 1000)
    {
        SerialDBG.println("long button press");
    }
    else
    {
        SerialDBG.println("short button press");
    }

    if (knx.progMode())
    {
        digitalWrite(knx.ledPin(), LOW);
        knx.progMode(false);
    }
    else
    {
        digitalWrite(knx.ledPin(), HIGH);
        knx.progMode(true);
    }

    attachInterrupt(knx.buttonPin(), buttonDown, FALLING);
}

KnxFacade::KnxFacade() : _bau(_platform)
{
    manufacturerId(0xfa);
}

bool KnxFacade::enabled()
{
    return _bau.enabled();
}

void KnxFacade::enabled(bool value)
{
    _bau.enabled(true);
}

bool KnxFacade::progMode()
{
    return _bau.deviceObject().progMode();
}

void KnxFacade::progMode(bool value)
{
    _bau.deviceObject().progMode(value);
}

bool KnxFacade::configured()
{
    return _bau.configured();
}

uint32_t KnxFacade::ledPin()
{
    return _ledPin;
}

void KnxFacade::ledPin(uint32_t value)
{
    _ledPin = value;
}

uint32_t KnxFacade::buttonPin()
{
    return _buttonPin;
}

void KnxFacade::buttonPin(uint32_t value)
{
    _buttonPin = value;
}

void KnxFacade::readMemory()
{
    _bau.readMemory();
}

void KnxFacade::loop()
{
    _bau.loop();
}

void KnxFacade::registerGroupObjects(GroupObject* groupObjects, uint16_t count)
{
    _bau.groupObjectTable().groupObjects(groupObjects, count);
}

void KnxFacade::manufacturerId(uint16_t value)
{
    _bau.deviceObject().manufacturerId(value);
}

void KnxFacade::bauNumber(uint32_t value)
{
    _bau.deviceObject().bauNumber(value);
}

void KnxFacade::orderNumber(const char* value)
{
    _bau.deviceObject().orderNumber(value);
}

void KnxFacade::hardwareType(uint8_t* value)
{
    _bau.deviceObject().hardwareType(value);
}

void KnxFacade::version(uint16_t value)
{
    _bau.deviceObject().version(value);
}

void KnxFacade::start()
{
    pinMode(_ledPin, OUTPUT);

    pinMode(_buttonPin, INPUT_PULLUP);
    attachInterrupt(_buttonPin, buttonDown, FALLING);
    enabled(true);
}

uint8_t* KnxFacade::paramData(uint32_t addr)
{
    if (!_bau.configured())
        return nullptr;
    
    return _bau.parameters().data(addr);
}

uint8_t KnxFacade::paramByte(uint32_t addr)
{
    if (!_bau.configured())
        return 0;

    return _bau.parameters().getByte(addr);
}

uint16_t KnxFacade::paramWord(uint32_t addr)
{
    if (!_bau.configured())
        return 0;

    return _bau.parameters().getWord(addr);
}

uint32_t KnxFacade::paramInt(uint32_t addr)
{
    if (!_bau.configured())
        return 0;

    return _bau.parameters().getInt(addr);
}


#endif