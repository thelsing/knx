#ifdef ARDUINO_ARCH_STM32
#include "arduino_platform.h"

class Stm32Platform : public ArduinoPlatform
{
public:
    Stm32Platform();
    Stm32Platform( HardwareSerial* s);
    ~Stm32Platform();

    // basic stuff
    void restart();
    
    //memory
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();
private:
	uint8_t *eepromPtr = nullptr;
	uint16_t eepromSize = 0;
};

#endif
