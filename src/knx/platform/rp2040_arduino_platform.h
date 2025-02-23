#pragma once

#include "arduino_platform.h"

#include "Arduino.h"

#ifdef ARDUINO_ARCH_RP2040

#ifndef USE_RP2040_EEPROM_EMULATION
#ifndef KNX_FLASH_OFFSET
#define KNX_FLASH_OFFSET 0x180000 // 1.5MiB
#pragma warning "KNX_FLASH_OFFSET not defined, using 0x180000"
#endif
#endif

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION
#define USE_RP2040_EEPROM_EMULATION
#endif

#ifndef KNX_SERIAL
#pragma warn "KNX_SERIAL not defined, using Serial1"
#define KNX_SERIAL Serial1
#endif

#ifdef KNX_IP_LAN
#if ARDUINO_PICO_MAJOR * 10000 + ARDUINO_PICO_MINOR * 100 + ARDUINO_PICO_REVISION < 30700
#pragma error "arduino-pico >= 3.7.0 needed"
#endif
#define KNX_NETIF Eth

#include "SPI.h"
#include <W5500lwIP.h>

#else
#include <WiFi.h>
#define KNX_NETIF WiFi
#endif

#if USE_KNX_DMA_UART == 1
#define KNX_DMA_UART uart1
#define KNX_DMA_UART_IRQ UART1_IRQ
#define KNX_DMA_UART_DREQ DREQ_UART1_RX
#else
#define KNX_DMA_UART uart0
#define KNX_DMA_UART_IRQ UART0_IRQ
#define KNX_DMA_UART_DREQ DREQ_UART0_RX
#endif

#if USE_KNX_DMA_IRQ == 1
#define KNX_DMA_IRQ DMA_IRQ_1
#else
#define KNX_DMA_IRQ DMA_IRQ_0
#endif

namespace Knx
{
    class RP2040ArduinoPlatform : public ArduinoPlatform
    {
        public:
            RP2040ArduinoPlatform();
            RP2040ArduinoPlatform(HardwareSerial* s);

            // uart
            void knxUartPins(pin_size_t rxPin, pin_size_t txPin);
            void setupUart() override;
            bool overflowUart() override;
#ifdef USE_KNX_DMA_UART
            int uartAvailable() override;
            void closeUart() override;
            void knxUart(HardwareSerial* serial) override {};
            HardwareSerial* knxUart() override
            {
                return nullptr;
            };
            size_t writeUart(const uint8_t data) override;
            size_t writeUart(const uint8_t* buffer, size_t size) override
            {
                return 0;
            };
            int readUart() override;
            size_t readBytesUart(uint8_t* buffer, size_t length) override
            {
                return 0;
            };
            void flushUart() override {};
#endif

            // unique serial number
            uint32_t uniqueSerialNumber() override;

            void restart();

#ifdef USE_RP2040_EEPROM_EMULATION
            uint8_t* getEepromBuffer(uint32_t size);
            void commitToEeprom();

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION
            uint8_t _rambuff[KNX_FLASH_SIZE];
            bool _rambuff_initialized = false;
#endif
#else

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
#endif

#if defined(KNX_NETIF)
            uint32_t currentIpAddress() override;
            uint32_t currentSubnetMask() override;
            uint32_t currentDefaultGateway() override;
            void macAddress(uint8_t* addr) override;

            // multicast
            void setupMultiCast(uint32_t addr, uint16_t port) override;
            void closeMultiCast() override;
            bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
            int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port) override;

            // unicast
            bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;

#define UDP_UNICAST _udp
        protected:
            WiFiUDP _udp;

        protected:
            IPAddress mcastaddr;

        protected:
            uint16_t _port;
#endif
        protected:
            pin_size_t _rxPin = UART_PIN_NOT_DEFINED;

        protected:
            pin_size_t _txPin = UART_PIN_NOT_DEFINED;

        protected:
            IPAddress _remoteIP = 0;

        protected:
            uint16_t _remotePort = 0;
    };
} // namespace Knx
#endif