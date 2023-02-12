#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_RP2040

#ifndef USE_RP2040_EEPROM_EMULATION
#ifndef KNX_FLASH_OFFSET
#define KNX_FLASH_OFFSET 0x180000   // 1.5MiB
#pragma warning "KNX_FLASH_OFFSET not defined, using 0x180000"
#endif
#endif

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION
#define USE_RP2040_EEPROM_EMULATION
#endif


class RP2040ArduinoPlatform : public ArduinoPlatform
{
public:
    RP2040ArduinoPlatform();
    RP2040ArduinoPlatform( HardwareSerial* s);

    void setupUart();

    // unique serial number
    uint32_t uniqueSerialNumber() override; 

    void restart();

    #ifdef USE_RP2040_EEPROM_EMULATION
    uint8_t* getEepromBuffer(uint32_t size);
    void commitToEeprom();

    #ifdef USE_RP2040_LARGE_EEPROM_EMULATION
    uint8_t _rambuff[KNX_FLASH_SIZE];
    bool _rambuff_initialized = false;
    #endif
    #else

    // size of one EraseBlock in pages
    virtual size_t flashEraseBlockSize();
    // size of one flash page in bytes
    virtual size_t flashPageSize();
    // start of user flash aligned to start of an erase block
    virtual uint8_t* userFlashStart();
    // size of the user flash in EraseBlocks
    virtual size_t userFlashSizeEraseBlocks();
    //relativ to userFlashStart
    virtual void flashErase(uint16_t eraseBlockNum);
    //write a single page to flash (pageNumber relative to userFashStart
    virtual void flashWritePage(uint16_t pageNumber, uint8_t* data); 
    
    // writes _eraseblockBuffer to flash - overrides Plattform::writeBufferedEraseBlock() for performance optimization only
    void writeBufferedEraseBlock();
    #endif
};

#endif
