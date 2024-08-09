#include "platform.h"

#include "bits.h"

#include <cstring>
#include <cstdlib>

NvMemoryType Platform::NonVolatileMemoryType()
{
    return _memoryType;
}

void Platform::NonVolatileMemoryType(NvMemoryType type)
{
    _memoryType = type;
}

void Platform::setupSpi()
{}

void Platform::closeSpi()
{}

int Platform::readWriteSpi(uint8_t *data, size_t len)
{
    return 0;
}

size_t Platform::readBytesUart(uint8_t *buffer, size_t length)
{
    return 0;
}

int Platform::readUart()
{
    return -1;
}

size_t Platform::writeUart(const uint8_t *buffer, size_t size)
{
    return 0;
}

size_t Platform::writeUart(const uint8_t data)
{
    return 0;
}

int Platform::uartAvailable()
{
    return 0;
}

void Platform::closeUart()
{}

void Platform::setupUart()
{}

bool Platform::overflowUart()
{
    return false;
}

void Platform::flushUart()
{}

uint32_t Platform::currentIpAddress()
{
    return 0x01020304;
}

uint32_t Platform::currentSubnetMask()
{
    return 0;
}

uint32_t Platform::currentDefaultGateway()
{
    return 0;
}

void Platform::macAddress(uint8_t *data)
{}

uint32_t Platform::uniqueSerialNumber()
{
    return 0x01020304;
}

void Platform::setupMultiCast(uint32_t addr, uint16_t port)
{}

void Platform::closeMultiCast()
{}

bool Platform::sendBytesMultiCast(uint8_t *buffer, uint16_t len)
{
    return false;
}

bool Platform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    return false;
}

int Platform::readBytesMultiCast(uint8_t *buffer, uint16_t maxLen)
{
    return 0;
}

int Platform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port)
{
    return readBytesMultiCast(buffer, maxLen);
}

size_t Platform::flashEraseBlockSize()
{
    return 0;
}

size_t Platform::flashPageSize()
{
    // align to 32bit as default for Eeprom Emulation plattforms
    return 4;
}

uint8_t *Platform::userFlashStart()
{
    return nullptr;
}

size_t Platform::userFlashSizeEraseBlocks()
{
    return 0;
}

void Platform::flashErase(uint16_t eraseBlockNum)
{}

void Platform::flashWritePage(uint16_t pageNumber, uint8_t* data)
{}

uint8_t * Platform::getEepromBuffer(uint32_t size)
{
    return nullptr;
}

void Platform::commitToEeprom()
{}

uint8_t* Platform::getNonVolatileMemoryStart()
{
    if(_memoryType == Flash)
        return userFlashStart();
#ifdef KNX_FLASH_CALLBACK
    else if(_memoryType == Callback)
        return _callbackFlashRead();
#endif
    else
        return getEepromBuffer(KNX_FLASH_SIZE);
}

size_t Platform::getNonVolatileMemorySize()
{
    if(_memoryType == Flash)
        return userFlashSizeEraseBlocks() * flashEraseBlockSize() * flashPageSize();
#ifdef KNX_FLASH_CALLBACK
    else if(_memoryType == Callback)
        return _callbackFlashSize();
#endif
    else
        return KNX_FLASH_SIZE;
}

void Platform::commitNonVolatileMemory()
{
    if(_memoryType == Flash)
    {
        if(_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
        {
            writeBufferedEraseBlock();
            
            free(_eraseblockBuffer);
            _eraseblockBuffer = nullptr;
            _bufferedEraseblockNumber = -1;  // does that make sense?
        }
    }
#ifdef KNX_FLASH_CALLBACK
    else if(_memoryType == Callback)
        return _callbackFlashCommit();
#endif
    else
    {
        commitToEeprom();
    }
}

uint32_t Platform::writeNonVolatileMemory(uint32_t relativeAddress, uint8_t* buffer, size_t size)
{
#ifdef KNX_LOG_MEM
    print("Platform::writeNonVolatileMemory relativeAddress ");
    print(relativeAddress);
    print(" size ");
    println(size);
#endif

    if(_memoryType == Flash)
    {
        while (size > 0)
        {
            loadEraseblockContaining(relativeAddress);
            uint32_t start = _bufferedEraseblockNumber * (flashEraseBlockSize() * flashPageSize());
            uint32_t end = start +  (flashEraseBlockSize() * flashPageSize());

            uint32_t offset = relativeAddress - start;
            uint32_t length = end - relativeAddress;
            if(length > size)
                length = size;
            memcpy(_eraseblockBuffer + offset, buffer, length);
            _bufferedEraseblockDirty = true;

            relativeAddress += length;
            buffer += length;
            size -= length;
        }
        return relativeAddress;
    }
#ifdef KNX_FLASH_CALLBACK
    else if(_memoryType == Callback)
        return _callbackFlashWrite(relativeAddress, buffer, size);
#endif
    else
    {
        memcpy(getEepromBuffer(KNX_FLASH_SIZE)+relativeAddress, buffer, size);
        return relativeAddress+size;
    }
}

uint32_t Platform::readNonVolatileMemory(uint32_t relativeAddress, uint8_t* buffer, size_t size)
{
#ifdef KNX_LOG_MEM
    print("Platform::readNonVolatileMemory relativeAddress ");
    print(relativeAddress);
    print(" size ");
    println(size);
#endif

    if(_memoryType == Flash)
    {
        uint32_t offset = 0;
        while (size > 0)
        {
            // bufferd block is "left" of requested memory, read until the end and return
            if(_bufferedEraseblockNumber < getEraseBlockNumberOf(relativeAddress))
            {
                memcpy(buffer+offset, userFlashStart()+relativeAddress, size);
                return relativeAddress + size;
            }
            // bufferd block is "right" of requested memory, and may interfere
            else if(_bufferedEraseblockNumber > getEraseBlockNumberOf(relativeAddress))
            {
                // if the end of the requested memory is before the buffered block, read until the end and return
                int32_t eraseblockNumberEnd = getEraseBlockNumberOf(relativeAddress+size-1);
                if(_bufferedEraseblockNumber > eraseblockNumberEnd)
                {
                    memcpy(buffer+offset, userFlashStart()+relativeAddress, size);
                    return relativeAddress + size;
                }
                // if not, read until the buffered block starts and loop through while again
                else
                {
                    uint32_t sizeToRead = (eraseblockNumberEnd * flashEraseBlockSize()) - relativeAddress;
                    memcpy(buffer+offset, userFlashStart()+relativeAddress, sizeToRead);
                    relativeAddress += sizeToRead;
                    size -= sizeToRead;
                    offset += sizeToRead;
                }
            }
            // start of requested memory is within the buffered erase block
            else
            {
                // if the end of the requested memory is also in the buffered block, read until the end and return
                int32_t eraseblockNumberEnd = getEraseBlockNumberOf(relativeAddress+size-1);
                if(_bufferedEraseblockNumber == eraseblockNumberEnd)
                {
                    uint8_t* start = _eraseblockBuffer + (relativeAddress - _bufferedEraseblockNumber * flashEraseBlockSize());
                    memcpy(buffer+offset, start, size);
                    return relativeAddress + size;
                }
                // if not, read until the end of the buffered block and loop through while again
                else
                {
                    uint32_t offsetInBufferedBlock = relativeAddress - _bufferedEraseblockNumber * flashEraseBlockSize();
                    uint8_t* start = _eraseblockBuffer + offsetInBufferedBlock;
                    uint32_t sizeToRead = flashEraseBlockSize() - offsetInBufferedBlock;
                    memcpy(buffer+offset, start, sizeToRead);
                    relativeAddress += sizeToRead;
                    size -= sizeToRead;
                    offset += sizeToRead;
                }
            }
        }
        return relativeAddress;
    }
    else
    {
        memcpy(buffer, getEepromBuffer(KNX_FLASH_SIZE)+relativeAddress, size);
        return relativeAddress+size;
    }
}

// writes value repeat times into flash starting at relativeAddress
// returns next free relativeAddress
uint32_t Platform::writeNonVolatileMemory(uint32_t relativeAddress, uint8_t value, size_t repeat)
{
    if(_memoryType == Flash)
    {
        while (repeat > 0)
        {
            loadEraseblockContaining(relativeAddress);
            uint32_t start = _bufferedEraseblockNumber * (flashEraseBlockSize() * flashPageSize());
            uint32_t end = start +  (flashEraseBlockSize() * flashPageSize());

            uint32_t offset = relativeAddress - start;
            uint32_t length = end - relativeAddress;
            if(length > repeat)
                length = repeat;
            memset(_eraseblockBuffer + offset, value, length);
            _bufferedEraseblockDirty = true;

            relativeAddress += length;
            repeat -= length;
        }
        return relativeAddress;
    }
    else
    {
        memset(getEepromBuffer(KNX_FLASH_SIZE)+relativeAddress, value, repeat);
        return relativeAddress+repeat;
    }
}

void Platform::loadEraseblockContaining(uint32_t relativeAddress)
{
    int32_t blockNum = getEraseBlockNumberOf(relativeAddress);
    if (blockNum < 0)
    {
        println("loadEraseblockContaining could not get valid eraseblock number");
        fatalError();
    }

    if (blockNum != _bufferedEraseblockNumber && _bufferedEraseblockNumber >= 0)
        writeBufferedEraseBlock();

    bufferEraseBlock(blockNum);
}

int32_t Platform::getEraseBlockNumberOf(uint32_t relativeAddress)
{
    return relativeAddress / (flashEraseBlockSize() * flashPageSize());
}


void Platform::writeBufferedEraseBlock()
{
    if(_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
    {
        flashErase(_bufferedEraseblockNumber);
        for(uint32_t i = 0; i < flashEraseBlockSize(); i++)
        {   
            int32_t pageNumber = _bufferedEraseblockNumber * flashEraseBlockSize() + i;
            uint8_t *data = _eraseblockBuffer + flashPageSize() * i;
            flashWritePage(pageNumber, data);
        }
        _bufferedEraseblockDirty = false;
    }
}


void Platform::bufferEraseBlock(int32_t eraseBlockNumber)
{
    if(_bufferedEraseblockNumber == eraseBlockNumber)
        return;
    
    if(_eraseblockBuffer == nullptr)
    {
        _eraseblockBuffer = (uint8_t*)malloc(flashEraseBlockSize() * flashPageSize());
    }
    memcpy(_eraseblockBuffer, userFlashStart() + eraseBlockNumber * flashEraseBlockSize() * flashPageSize(), flashEraseBlockSize() * flashPageSize());

    _bufferedEraseblockNumber = eraseBlockNumber;
    _bufferedEraseblockDirty = false;
}


#ifdef KNX_FLASH_CALLBACK
void Platform::registerFlashCallbacks(
    FlashCallbackSize callbackFlashSize,
    FlashCallbackRead callbackFlashRead,
    FlashCallbackWrite callbackFlashWrite,
    FlashCallbackCommit callbackFlashCommit)
    {
        println("Set Callback");
        _memoryType = Callback;
        _callbackFlashSize = callbackFlashSize;
        _callbackFlashRead = callbackFlashRead;
        _callbackFlashWrite = callbackFlashWrite;
        _callbackFlashCommit = callbackFlashCommit;
        _callbackFlashSize();
    }

FlashCallbackSize Platform::callbackFlashSize()
{
   return _callbackFlashSize;
}
FlashCallbackRead Platform::callbackFlashRead()
{
   return _callbackFlashRead;
}
FlashCallbackWrite Platform::callbackFlashWrite()
{
   return _callbackFlashWrite;
}
FlashCallbackCommit Platform::callbackFlashCommit()
{
   return _callbackFlashCommit;
}
#endif
