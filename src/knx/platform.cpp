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
    else
        return getEepromBuffer(KNX_FLASH_SIZE);
}

size_t Platform::getNonVolatileMemorySize()
{
    if(_memoryType == Flash)
        return userFlashSizeEraseBlocks() * flashEraseBlockSize() * flashPageSize();
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
    else
    {
        commitToEeprom();
    }
}

uint32_t Platform::writeNonVolatileMemory(uint32_t relativeAddress, uint8_t* buffer, size_t size)
{
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
    else
    {
        memcpy(getEepromBuffer(KNX_FLASH_SIZE)+relativeAddress, buffer, size);
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
