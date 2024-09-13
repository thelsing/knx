#include "samd21_platform.h"
#if defined(_SAMD21_)
#include <knx/bits.h>

#include <Arduino.h>
#ifdef USE_SAMD_EEPROM_EMULATION
    #include <FlashAsEEPROM.h>
#endif

#if KNX_FLASH_SIZE % 1024
    #error "KNX_FLASH_SIZE must be multiple of 1024"
#endif

#ifndef KNX_SERIAL
    #define KNX_SERIAL Serial1
#endif

extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;

namespace Knx
{

    Samd21Platform::Samd21Platform()
#ifndef KNX_NO_DEFAULT_UART
        : ArduinoPlatform(&KNX_SERIAL)
#endif
    {
#ifndef USE_SAMD_EEPROM_EMULATION
        init();
#endif
    }

    Samd21Platform::Samd21Platform( HardwareSerial* s) : ArduinoPlatform(s)
    {
#ifndef USE_SAMD_EEPROM_EMULATION
        init();
#endif
    }

    uint32_t Samd21Platform::uniqueSerialNumber()
    {
        // SAMD21 from section 9.3.3 of the datasheet
#define SERIAL_NUMBER_WORD_0  *(volatile uint32_t*)(0x0080A00C)
#define SERIAL_NUMBER_WORD_1  *(volatile uint32_t*)(0x0080A040)
#define SERIAL_NUMBER_WORD_2  *(volatile uint32_t*)(0x0080A044)
#define SERIAL_NUMBER_WORD_3  *(volatile uint32_t*)(0x0080A048)

        return SERIAL_NUMBER_WORD_0 ^ SERIAL_NUMBER_WORD_1 ^ SERIAL_NUMBER_WORD_2 ^ SERIAL_NUMBER_WORD_3;
    }

    void Samd21Platform::restart()
    {
        println("restart");
        NVIC_SystemReset();
    }

#ifdef USE_SAMD_EEPROM_EMULATION
#pragma warning "Using EEPROM Simulation"
    uint8_t* Samd21Platform::getEepromBuffer(uint32_t size)
    {
        //EEPROM.begin(size);
        if (size > EEPROM_EMULATION_SIZE)
            fatalError();

        return EEPROM.getDataPtr();
    }

    void Samd21Platform::commitToEeprom()
    {
        EEPROM.commit();
    }
#else

    static const uint32_t pageSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};

    void Samd21Platform::init()
    {
        _memoryType = Flash;
        _pageSize = pageSizes[NVMCTRL->PARAM.bit.PSZ];
        _pageCnt = NVMCTRL->PARAM.bit.NVMP;
        _rowSize = PAGES_PER_ROW * _pageSize;

        // find end of program flash and set limit to next row
        uint32_t endEddr = (uint32_t)(&__etext + (&__data_end__ - &__data_start__)); // text + data MemoryBlock
#ifdef KNX_FLASH_OFFSET
        _MemoryStart = KNX_FLASH_OFFSET;
        _MemoryEnd = KNX_FLASH_OFFSET + KNX_FLASH_SIZE;
#else
        _MemoryStart = getRowAddr(_pageSize * _pageCnt - KNX_FLASH_SIZE - 1);        // 23295
        _MemoryEnd = getRowAddr(_pageSize * _pageCnt - 1);
#endif

        // chosen flash size is not available anymore
        if (_MemoryStart < endEddr)
        {
            println("KNX_FLASH_SIZE is not available (possible too much flash use by firmware)");
            fatalError();
        }
    }

    size_t Samd21Platform::flashEraseBlockSize()
    {
        return PAGES_PER_ROW;
    }

    size_t Samd21Platform::flashPageSize()
    {
        return _pageSize;
    }

    uint8_t* Samd21Platform::userFlashStart()
    {
        return (uint8_t*)_MemoryStart;
    }

    size_t Samd21Platform::userFlashSizeEraseBlocks()
    {
        if (KNX_FLASH_SIZE <= 0)
            return 0;
        else
            return ((KNX_FLASH_SIZE - 1) / (flashPageSize() * flashEraseBlockSize())) + 1;
    }

    void Samd21Platform::flashErase(uint16_t eraseBlockNum)
    {
        noInterrupts();

        eraseRow((void*)(_MemoryStart + eraseBlockNum * _rowSize));
        // flash_range_erase(KNX_FLASH_OFFSET + eraseBlockNum * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());

        interrupts();
    }

    void Samd21Platform::flashWritePage(uint16_t pageNumber, uint8_t* data)
    {
        noInterrupts();

        write((void*)(_MemoryStart + pageNumber * _pageSize), data, _pageSize);
        // flash_range_program(KNX_FLASH_OFFSET + pageNumber * flashPageSize(), data, flashPageSize());

        interrupts();
    }

    void Samd21Platform::writeBufferedEraseBlock()
    {
        if (_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
        {
            noInterrupts();

            eraseRow((void*)(_MemoryStart + _bufferedEraseblockNumber * _rowSize));
            write((void*)(_MemoryStart + _bufferedEraseblockNumber * _rowSize), _eraseblockBuffer, _rowSize);
            // flash_range_erase(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());
            // flash_range_program(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), _eraseblockBuffer, flashPageSize() * flashEraseBlockSize());

            interrupts();

            _bufferedEraseblockDirty = false;
        }
    }

    uint32_t Samd21Platform::getRowAddr(uint32_t flasAddr)
    {
        return flasAddr & ~(_rowSize - 1);
    }

    void Samd21Platform::write(const volatile void* flash_ptr, const void* data, uint32_t size)
    {
        // Calculate data boundaries
        size = (size + 3) / 4;
        volatile uint32_t* src_addr = (volatile uint32_t*)data;
        volatile uint32_t* dst_addr = (volatile uint32_t*)flash_ptr;
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

    void Samd21Platform::erase(const volatile void* flash_ptr, uint32_t size)
    {
        const uint8_t* ptr = (const uint8_t*)flash_ptr;

        while (size > _rowSize)
        {
            eraseRow(ptr);
            ptr += _rowSize;
            size -= _rowSize;
        }

        eraseRow(ptr);
    }

    void Samd21Platform::eraseRow(const volatile void* flash_ptr)
    {
        NVMCTRL->ADDR.reg = ((uint32_t)flash_ptr) / 2;
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;

        while (!NVMCTRL->INTFLAG.bit.READY)
        {
        }
    }

#endif
}
#endif