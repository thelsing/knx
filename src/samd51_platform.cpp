#ifdef __SAMD51__
#include <Arduino.h>
#include "samd51_platform.h"
#include <knx/bits.h>

#if KNX_FLASH_SIZE % 1024
#error "KNX_FLASH_SIZE must be multiple of 1024"
#endif

#ifndef KNX_SERIAL
#define KNX_SERIAL Serial1
#endif

Samd51Platform::Samd51Platform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&KNX_SERIAL)
#endif
{
    init();
}

Samd51Platform::Samd51Platform( HardwareSerial* s) : ArduinoPlatform(s)
{
    init();
}

uint32_t Samd51Platform::uniqueSerialNumber()
{
	// SAMD51 from section 9.6 of the datasheet
	#define SERIAL_NUMBER_WORD_0	*(volatile uint32_t*)(0x008061FC)
	#define SERIAL_NUMBER_WORD_1	*(volatile uint32_t*)(0x00806010)
	#define SERIAL_NUMBER_WORD_2	*(volatile uint32_t*)(0x00806014)
	#define SERIAL_NUMBER_WORD_3	*(volatile uint32_t*)(0x00806018)

    return SERIAL_NUMBER_WORD_0 ^ SERIAL_NUMBER_WORD_1 ^ SERIAL_NUMBER_WORD_2 ^ SERIAL_NUMBER_WORD_3;
}

void Samd51Platform::restart()
{
    println("restart");
    NVIC_SystemReset();
}

extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;

static const uint32_t pageSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};

void Samd51Platform::init()
{
    _memoryType = Flash;
    _pageSize = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    _pageCnt = NVMCTRL->PARAM.bit.NVMP;
    _rowSize = (_pageSize * _pageCnt / 64);

    //find end of program flash and set limit to next row
    uint32_t endEddr = (uint32_t)(&__etext + (&__data_end__ - &__data_start__)); // text + data MemoryBlock
    _MemoryStart = getRowAddr(_pageSize * _pageCnt - KNX_FLASH_SIZE - 1);        // 23295
    _MemoryEnd = getRowAddr(_pageSize * _pageCnt - 1);
    //chosen flash size is not available anymore
    if (_MemoryStart < endEddr) {
        println("KNX_FLASH_SIZE is not available (possible too much flash use by firmware)");
        fatalError();
    }
}

// Invalidate all CMCC cache entries if CMCC cache is enabled.
static void invalidate_CMCC_cache()
{
	if (CMCC->SR.bit.CSTS) {
		CMCC->CTRL.bit.CEN = 0;
	while (CMCC->SR.bit.CSTS) {}
		CMCC->MAINT0.bit.INVALL = 1;
		CMCC->CTRL.bit.CEN = 1;
	}
}

static inline uint32_t read_unaligned_uint32(volatile void *data)
{
	union {
	uint32_t u32;
	uint8_t u8[4];
	} res;
	const uint8_t *d = (const uint8_t *)data;
	res.u8[0] = d[0];
	res.u8[1] = d[1];
	res.u8[2] = d[2];
	res.u8[3] = d[3];
	return res.u32;
}

size_t Samd51Platform::flashEraseBlockSize()
{
    return (_pageSize / 64);   //PAGES_PER_ROW;
}

size_t Samd51Platform::flashPageSize()
{
    return _pageSize;
}

uint8_t* Samd51Platform::userFlashStart()
{
    return (uint8_t*)_MemoryStart;
}

size_t Samd51Platform::userFlashSizeEraseBlocks()
{
    if (KNX_FLASH_SIZE <= 0)
        return 0;
    else
        return ((KNX_FLASH_SIZE - 1) / (flashPageSize() * flashEraseBlockSize())) + 1;
}

void Samd51Platform::flashErase(uint16_t eraseBlockNum)
{
    noInterrupts();

    eraseRow((void *)(_MemoryStart + eraseBlockNum * _rowSize));
    // flash_range_erase(KNX_FLASH_OFFSET + eraseBlockNum * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());

    interrupts();
}

void Samd51Platform::flashWritePage(uint16_t pageNumber, uint8_t* data)
{
    noInterrupts();

    write((void *)(_MemoryStart + pageNumber * _pageSize), data, _pageSize);
    // flash_range_program(KNX_FLASH_OFFSET + pageNumber * flashPageSize(), data, flashPageSize());

    interrupts();
}

void Samd51Platform::writeBufferedEraseBlock()
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

uint32_t Samd51Platform::getRowAddr(uint32_t flasAddr)
{
    return flasAddr & ~(_rowSize - 1);
}

void Samd51Platform::write(const volatile void *flash_ptr, const void *data, uint32_t size)
{
    // Calculate data boundaries
    size = (size + 3) / 4;
    volatile uint32_t *src_addr = (volatile uint32_t *)data;
    volatile uint32_t *dst_addr = (volatile uint32_t *)flash_ptr;

    // Disable automatic page write
	NVMCTRL->CTRLA.bit.WMODE = 0;
	while (NVMCTRL->STATUS.bit.READY == 0) { }
	// Disable NVMCTRL cache while writing, per SAMD51 errata.
	bool original_CACHEDIS0 = NVMCTRL->CTRLA.bit.CACHEDIS0;
	bool original_CACHEDIS1 = NVMCTRL->CTRLA.bit.CACHEDIS1;
	NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
	NVMCTRL->CTRLA.bit.CACHEDIS1 = true;

    // Do writes in pages
    while (size)
    {
        // Execute "PBC" Page Buffer Clear
		NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
		while (NVMCTRL->INTFLAG.bit.DONE == 0) { }

        // Fill page buffer
        uint32_t i;
        for (i = 0; i < (_pageSize / 4) && size; i++)
        {
            *dst_addr = read_unaligned_uint32(src_addr);
            src_addr += 4;
            dst_addr++;
            size--;
        }

        // Execute "WP" Write Page
		NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WP;
		while (NVMCTRL->INTFLAG.bit.DONE == 0) { }
		invalidate_CMCC_cache();
		// Restore original NVMCTRL cache settings.
		NVMCTRL->CTRLA.bit.CACHEDIS0 = original_CACHEDIS0;
		NVMCTRL->CTRLA.bit.CACHEDIS1 = original_CACHEDIS1;
    }
}

void Samd51Platform::erase(const volatile void *flash_ptr, uint32_t size)
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

void Samd51Platform::eraseRow(const volatile void *flash_ptr)
{
	NVMCTRL->ADDR.reg = ((uint32_t)flash_ptr);
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
	while (!NVMCTRL->INTFLAG.bit.DONE) { }
	invalidate_CMCC_cache();
}

#endif
