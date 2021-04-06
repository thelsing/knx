#include "platform.h"


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
