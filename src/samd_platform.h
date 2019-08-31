#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_SAMD

#define SerialDBG SerialUSB

class SamdPlatform : public ArduinoPlatform
{
public:
    SamdPlatform();
    SamdPlatform( HardwareSerial& s);

    void restart();
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();
};

#endif