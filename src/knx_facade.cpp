#include "knx_facade.h"

#ifdef ARDUINO_ARCH_SAMD
SamdPlatform platform;
Bau07B0 bau(platform);
#else
EspPlatform platform;
Bau57B0 bau(platform);
#endif
KnxFacade knx(bau);

void buttonUp()
{
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
}

KnxFacade::KnxFacade(BauSystemB& bau) : _bau(bau)
{
    manufacturerId(0xfa);
    _bau.addSaveRestore(this);
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

void KnxFacade::writeMemory()
{
    _bau.writeMemory();
}

void KnxFacade::loop()
{
    _bau.loop();
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
    
    attachInterrupt(_buttonPin, buttonUp, RISING);
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


void KnxFacade::setSaveCallback(saveRestoreCallback func)
{
    _saveCallback = func;
}


void KnxFacade::setRestoreCallback(saveRestoreCallback func)
{
    _restoreCallback = func;
}


uint8_t* KnxFacade::save(uint8_t* buffer)
{
    if (_saveCallback != 0)
        return _saveCallback(buffer);
    
    return buffer;
}


uint8_t* KnxFacade::restore(uint8_t* buffer)
{
    if (_restoreCallback != 0)
        return _restoreCallback(buffer);
    
    return buffer;
}


GroupObject& KnxFacade::getGroupObject(uint16_t goNr)
{
    return _bau.groupObjectTable().get(goNr);
}
