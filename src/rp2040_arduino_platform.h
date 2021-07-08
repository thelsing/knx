#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_RP2040

class RP2040ArduinoPlatform : public ArduinoPlatform
{
public:
    RP2040ArduinoPlatform();
    RP2040ArduinoPlatform( HardwareSerial* s);

    // unique serial number
    uint32_t uniqueSerialNumber() override;

    void restart();
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();
};

#endif
