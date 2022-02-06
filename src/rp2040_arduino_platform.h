#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_RP2040

#ifndef KNX_FLASH_OFFSET
#define KNX_FLASH_OFFSET 0x180000   // 1.5MiB
#endif

#ifndef KNX_FLASH_SIZE
#define KNX_FLASH_SIZE 1024
#endif

class RP2040ArduinoPlatform : public ArduinoPlatform
{
public:
    RP2040ArduinoPlatform();
    RP2040ArduinoPlatform( HardwareSerial* s);

    // unique serial number
    uint32_t uniqueSerialNumber(); //override; 

    void restart();
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();



    size_t flashEraseBlockSize(); // in bytes
    size_t flashPageSize();       // in bytes
    uint8_t* userFlashStart();   // start of user flash aligned to start of an erase block
    size_t userFlashSizeEraseBlocks(); // in eraseBlocks
    void flashErase(uint16_t eraseBlockNum); //relativ to userFlashStart
    void flashWritePage(uint16_t pageNumber, uint8_t* data); //write a single page to flash (pageNumber relative to userFashStart
};

#endif
