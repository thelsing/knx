#include <Arduino.h>
#include "arduino_platform.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#ifdef __SAMD51__

class Samd51Platform : public ArduinoPlatform
{
public:
    Samd51Platform();
    Samd51Platform( HardwareSerial* s);

    // unique serial number
    uint32_t uniqueSerialNumber() override;

    void restart();

    #if USE_W5X00 == 1
        // ip config
        uint32_t currentIpAddress() override;
        uint32_t currentSubnetMask() override;
        uint32_t currentDefaultGateway() override;
        void macAddress(uint8_t* data) override;

        //multicast
        void setupMultiCast(uint32_t addr, uint16_t port) override;
        void closeMultiCast() override;
        bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
        int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen) override;

        bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;
    #endif

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

    #if USE_W5X00 == 1
        uint8_t _macAddress[6] = {0xC0, 0xFF, 0xEE, 0xC0, 0xDE, 0x00};
        uint32_t _ipAddress = 0;
        uint32_t _netmask = 0;
        uint32_t _defaultGateway = 0;

        uint32_t _multicastAddr = -1;
        uint16_t _multicastPort = -1;
        EthernetUDP _udp;
    #endif
};

#endif
