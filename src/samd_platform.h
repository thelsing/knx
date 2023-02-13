#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_SAMD

#define PAGES_PER_ROW 4

class SamdPlatform : public ArduinoPlatform
{
public:
    SamdPlatform();
    SamdPlatform( HardwareSerial* s);

    // unique serial number
    uint32_t uniqueSerialNumber() override;

    void restart();
#ifdef USE_SAMD_EEPROM_EMULATION
    uint8_t* getEepromBuffer(uint32_t size);
    void commitToEeprom();
#else
    // size of one EraseBlock in pages
    virtual size_t flashEraseBlockSize();
    // size of one flash page in bytes
    virtual size_t flashPageSize();
    // start of user flash aligned to start of an erase block
    virtual uint8_t* userFlashStart();
    // size of the user flash in EraseBlocks
    virtual size_t userFlashSizeEraseBlocks();
    // relativ to userFlashStart
    virtual void flashErase(uint16_t eraseBlockNum);
    // write a single page to flash (pageNumber relative to userFashStart
    virtual void flashWritePage(uint16_t pageNumber, uint8_t* data);

    // writes _eraseblockBuffer to flash - overrides Plattform::writeBufferedEraseBlock() for performance optimization only
    void writeBufferedEraseBlock();

private:
  void init();
  uint32_t _MemoryEnd = 0;
  uint32_t _MemoryStart = 0;
  uint32_t _pageSize;
  uint32_t _rowSize;
  uint32_t _pageCnt;

  uint32_t getRowAddr(uint32_t flasAddr);
  void write(const volatile void* flash_ptr, const void* data, uint32_t size);
  void erase(const volatile void* flash_ptr, uint32_t size);
  void eraseRow(const volatile void* flash_ptr);

#endif
};

#endif
