#include "samd_platform.h"

#ifdef ARDUINO_ARCH_SAMD
#include <knx/bits.h>

#include <Arduino.h>
#ifdef USE_SAMD_EEPROM_EMULATION
#include <FlashAsEEPROM.h>
#endif

#if KNX_FLASH_SIZE % 1024
#error "KNX_FLASH_SIZE must be multiple of 1024"
#endif

SamdPlatform::SamdPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&Serial1)
#endif
{
#ifndef USE_SAMD_EEPROM_EMULATION
    init();
#endif
}

SamdPlatform::SamdPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
#ifndef USE_SAMD_EEPROM_EMULATION
    init();
#endif
}

uint32_t SamdPlatform::uniqueSerialNumber()
{
    #if defined (__SAMD51__)
      // SAMD51 from section 9.6 of the datasheet
      #define SERIAL_NUMBER_WORD_0	*(volatile uint32_t*)(0x008061FC)
      #define SERIAL_NUMBER_WORD_1	*(volatile uint32_t*)(0x00806010)
      #define SERIAL_NUMBER_WORD_2	*(volatile uint32_t*)(0x00806014)
      #define SERIAL_NUMBER_WORD_3	*(volatile uint32_t*)(0x00806018)
    #else
    //#elif defined (__SAMD21E17A__) || defined(__SAMD21G18A__)  || defined(__SAMD21E18A__) || defined(__SAMD21J18A__)
    // SAMD21 from section 9.3.3 of the datasheet
      #define SERIAL_NUMBER_WORD_0	*(volatile uint32_t*)(0x0080A00C)
      #define SERIAL_NUMBER_WORD_1	*(volatile uint32_t*)(0x0080A040)
      #define SERIAL_NUMBER_WORD_2	*(volatile uint32_t*)(0x0080A044)
      #define SERIAL_NUMBER_WORD_3	*(volatile uint32_t*)(0x0080A048)
    #endif

    return SERIAL_NUMBER_WORD_0 ^ SERIAL_NUMBER_WORD_1 ^ SERIAL_NUMBER_WORD_2 ^ SERIAL_NUMBER_WORD_3;
}

void SamdPlatform::restart()
{
    println("restart");
    NVIC_SystemReset();
}

#ifdef USE_SAMD_EEPROM_EMULATION
#pragma warning "Using EEPROM Simulation"
uint8_t* SamdPlatform::getEepromBuffer(uint16_t size)
{
    //EEPROM.begin(size);
    if(size > EEPROM_EMULATION_SIZE)
        fatalError();
    
    return EEPROM.getDataPtr();
}

void SamdPlatform::commitToEeprom()
{
    EEPROM.commit();
}
#else

extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;

static const uint32_t pageSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};

void SamdPlatform::init()
{
    _memoryType = Flash;
    _pageSize = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    _pageCnt = NVMCTRL->PARAM.bit.NVMP;
    _rowSize = PAGES_PER_ROW * _pageSize;

    // find end of program flash and set limit to next row
    uint32_t endEddr = (uint32_t)(&__etext + (&__data_end__ - &__data_start__)); // text + data MemoryBlock
    _MemoryStart = getRowAddr(_pageSize * _pageCnt - KNX_FLASH_SIZE - 1);        // 23295
    _MemoryEnd = getRowAddr(_pageSize * _pageCnt - 1);
    // chosen flash size is not available anymore
    if (_MemoryStart < endEddr) {
        println("KNX_FLASH_SIZE is not available (possible too much flash use by firmware)");
        fatalError();
    }
}

size_t SamdPlatform::flashEraseBlockSize()
{
    return PAGES_PER_ROW;
}

size_t SamdPlatform::flashPageSize()
{
    return _pageSize;
}

uint8_t* SamdPlatform::userFlashStart()
{
    return (uint8_t*)_MemoryStart;
}

size_t SamdPlatform::userFlashSizeEraseBlocks()
{
    if (KNX_FLASH_SIZE <= 0)
        return 0;
    else
        return ((KNX_FLASH_SIZE - 1) / (flashPageSize() * flashEraseBlockSize())) + 1;
}

void SamdPlatform::flashErase(uint16_t eraseBlockNum)
{
    noInterrupts();

    eraseRow((void *)(_MemoryStart + eraseBlockNum * _rowSize));
    // flash_range_erase(KNX_FLASH_OFFSET + eraseBlockNum * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());

    interrupts();
}

void SamdPlatform::flashWritePage(uint16_t pageNumber, uint8_t* data)
{
    noInterrupts();

    write((void *)(_MemoryStart + pageNumber * _pageSize), data, _pageSize);
    // flash_range_program(KNX_FLASH_OFFSET + pageNumber * flashPageSize(), data, flashPageSize());

    interrupts();
}

void SamdPlatform::writeBufferedEraseBlock()
{
    if (_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
    {
        noInterrupts();

        eraseRow((void *)(_MemoryStart + _bufferedEraseblockNumber * _rowSize));
        write((void *)(_MemoryStart + _bufferedEraseblockNumber * _rowSize), _eraseblockBuffer, _rowSize);
        // flash_range_erase(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());
        // flash_range_program(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), _eraseblockBuffer, flashPageSize() * flashEraseBlockSize());

        interrupts();

        _bufferedEraseblockDirty = false;
    }
}

uint32_t SamdPlatform::getRowAddr(uint32_t flasAddr)
{
    return flasAddr & ~(_rowSize - 1);
}

void SamdPlatform::write(const volatile void *flash_ptr, const void *data, uint32_t size)
{
    // Calculate data boundaries
    size = (size + 3) / 4;
    volatile uint32_t *src_addr = (volatile uint32_t *)data;
    volatile uint32_t *dst_addr = (volatile uint32_t *)flash_ptr;
    // volatile uint32_t *dst_addr = (volatile uint32_t *)flash_ptr;
    // const uint8_t *src_addr = (uint8_t *)data;

    // Disable automatic page write
    NVMCTRL->CTRLB.bit.MANW = 1;

    // Do writes in pages
    while (size)
    {
        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        while (NVMCTRL->INTFLAG.bit.READY == 0)
        {
        }

        // Fill page buffer
        uint32_t i;
        for (i = 0; i < (_pageSize / 4) && size; i++)
        {
            *dst_addr = *src_addr;
            src_addr++;
            dst_addr++;
            size--;
        }

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        while (NVMCTRL->INTFLAG.bit.READY == 0)
        {
        }
    }
}

void SamdPlatform::erase(const volatile void *flash_ptr, uint32_t size)
{
    const uint8_t *ptr = (const uint8_t *)flash_ptr;
    while (size > _rowSize)
    {
        eraseRow(ptr);
        ptr += _rowSize;
        size -= _rowSize;
    }
    eraseRow(ptr);
}

void SamdPlatform::eraseRow(const volatile void *flash_ptr)
{
    NVMCTRL->ADDR.reg = ((uint32_t)flash_ptr) / 2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
    while (!NVMCTRL->INTFLAG.bit.READY)
    {
    }
}

#endif
#endif
