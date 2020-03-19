#include "platform.h"

#include "bits.h"

#include <cstring>

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
    return 0;
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

uint8_t* Platform::getNonVolatileMemoryStart()
{
    return userFlashStart();
}

size_t Platform::getNonVolatileMemorySize()
{
    return userFlashSizeEraseBlocks() * flashEraseBlockSize() * flashPageSize();
}

void Platform::commitNonVolatileMemory()
{
}

uint32_t Platform::writeNonVolatileMemory(uint32_t relativeAddress, uint8_t* buffer, size_t size)
{
    while (size > 0)
    {
        loadEraseblockContaining(relativeAddress);
        uint32_t start = bufferedEraseBlockStart();
        uint32_t end = bufferedEraseBlockEnd();

        ptrdiff_t offset = relativeAddress - start;
        ptrdiff_t length = end - relativeAddress;
        memcpy(_eraseblockBuffer + offset, buffer, length);
        _bufferedEraseblockDirty = true;

        relativeAddress += length;
        buffer += length;
        size -= length;
    }
    return relativeAddress;
}

void Platform::loadEraseblockContaining(uint32_t relativeAddress)
{
    int32_t blockNum = getEraseBlockNumberOf(relativeAddress);
    if (blockNum < 0)
    {
        println("loadEraseblockContaining could not get valid eraseblock number");
        fatalError();
    }

    if (blockNum != _bufferedEraseblockNumber)
        writeBufferedEraseBlock();

    bufferEraseBlock(blockNum);
}

uint32_t Platform::bufferedEraseBlockStart()
{
    return  _bufferedEraseblockNumber * flashEraseBlockSize();
}

uint32_t Platform::bufferedEraseBlockEnd()
{
    return (_bufferedEraseblockNumber + 1) * flashEraseBlockSize() -1;
}


int32_t Platform::getEraseBlockNumberOf(uint32_t relativeAddress)
{
    return -1;
}


void Platform::writeBufferedEraseBlock()
{
}


void Platform::bufferEraseBlock(uint32_t eraseBlockNumber)
{
}
