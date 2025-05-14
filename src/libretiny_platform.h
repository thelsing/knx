#ifdef LIBRETINY
#include "arduino_platform.h"

#include <WiFi.h>
#include <LwIPUdp.h>

class LibretinyPlatform : public ArduinoPlatform
{
    public:
        LibretinyPlatform();
        LibretinyPlatform(HardwareSerial* s);

        // ip stuff
        uint32_t currentIpAddress() override;
        uint32_t currentSubnetMask() override;
        uint32_t currentDefaultGateway() override;
        void macAddress(uint8_t* addr) override;

        // unique serial number
        uint32_t uniqueSerialNumber() override;

        // basic stuff
        void restart();

        //multicast
        void setupMultiCast(uint32_t addr, uint16_t port) override;
        void closeMultiCast() override;
        bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
        int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen) override;

        //unicast
        bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;

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
        WiFiUDP _udp;
};

#endif
