#include "libretiny_platform.h"

#ifdef LIBRETINY
#include <Arduino.h>
#include "knx/bits.h"

#include <lwip/netif.h>

#ifndef KNX_SERIAL
#define KNX_SERIAL Serial
#endif

#ifndef KNX_FLASH_OFFSET
#error "KNX_FLASH_OFFSET is not defined. E.g. 0x1DB000 for BK7231N"
#elif (KNX_FLASH_OFFSET % 4096) != 0
#error "KNX_FLASH_OFFSET must be a multiple of 4096"
#endif

static uint8_t NVS_buffer[KNX_FLASH_SIZE];

LibretinyPlatform::LibretinyPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : ArduinoPlatform(&KNX_SERIAL)
#endif
{
    _memoryType = Flash;
}

LibretinyPlatform::LibretinyPlatform( HardwareSerial* s) : ArduinoPlatform(s)
{
    _memoryType = Flash;
}

uint32_t LibretinyPlatform::currentIpAddress()
{
    return WiFi.localIP();
}

uint32_t LibretinyPlatform::currentSubnetMask()
{
    return WiFi.subnetMask();
}

uint32_t LibretinyPlatform::currentDefaultGateway()
{
    return WiFi.gatewayIP();
}

void LibretinyPlatform::macAddress(uint8_t * addr)
{
    WiFi.macAddress(addr);
}

uint32_t LibretinyPlatform::uniqueSerialNumber()
{
    return lt_cpu_get_mac_id();
}

void LibretinyPlatform::restart()
{
    println("restart");
    lt_reboot();
}

void LibretinyPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    //workaround for libretiny bug: NETIF_FLAG_IGMP is not set by default
    struct netif *netif;
    for (netif = netif_list; netif != NULL; netif = netif->next)
    {
        netif->flags |= NETIF_FLAG_IGMP;
    }

    IPAddress mcastaddr(htonl(addr));
    KNX_DEBUG_SERIAL.printf("setup multicast addr: %d.%d.%d.%d port: %d ip: %d.%d.%d.%d\n", mcastaddr[0], mcastaddr[1], mcastaddr[2], mcastaddr[3], port, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    uint8_t result = _udp.beginMulticast(mcastaddr, port);
    KNX_DEBUG_SERIAL.printf("multicast setup result %d\n", result);
}

void LibretinyPlatform::closeMultiCast()
{
    _udp.stop();
}

bool LibretinyPlatform::sendBytesMultiCast(uint8_t * buffer, uint16_t len)
{
    _udp.beginMulticastPacket();
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}

int LibretinyPlatform::readBytesMultiCast(uint8_t * buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();

    if (len == 0)
        return 0;

    if (len > maxLen)
    {
        KNX_DEBUG_SERIAL.printf("udp buffer to small. was %d, needed %d\n", maxLen, len);
        fatalError();
    }

    _udp.read(buffer, len);
    return len;
}

bool LibretinyPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    IPAddress ucastaddr(htonl(addr));
    println("sendBytesUniCast endPacket fail");

    if(_udp.beginPacket(ucastaddr, port) == 1)
    {
        _udp.write(buffer, len);

        if(_udp.endPacket() == 0)
            println("sendBytesUniCast endPacket fail");
    }
    else
        println("sendBytesUniCast beginPacket fail");

    return true;
}

size_t LibretinyPlatform::flashEraseBlockSize()
{
    return 16;
}

size_t LibretinyPlatform::flashPageSize()
{
    return 256;
}

uint8_t* LibretinyPlatform::userFlashStart()
{
    lt_flash_read(KNX_FLASH_OFFSET, NVS_buffer, KNX_FLASH_SIZE);
    return NVS_buffer;
}

size_t LibretinyPlatform::userFlashSizeEraseBlocks()
{
    if(KNX_FLASH_SIZE <= 0)
        return 0;
    else
        return ( (KNX_FLASH_SIZE - 1) / (flashPageSize() * flashEraseBlockSize())) + 1;
}

void LibretinyPlatform::flashErase(uint16_t eraseBlockNum)
{
    // 16 pages x 256byte/page = 4096byte
    lt_flash_erase_block(KNX_FLASH_OFFSET + eraseBlockNum * flashPageSize() * flashEraseBlockSize());
}

void LibretinyPlatform::flashWritePage(uint16_t pageNumber, uint8_t* data)
{
    lt_flash_write(KNX_FLASH_OFFSET + pageNumber * flashPageSize(), data, flashPageSize());
}

void LibretinyPlatform::writeBufferedEraseBlock()
{
    if(_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
    {
        lt_flash_erase_block(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize());
        lt_flash_write(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), _eraseblockBuffer, flashPageSize() * flashEraseBlockSize());
        _bufferedEraseblockDirty = false;
    }
}

#endif
