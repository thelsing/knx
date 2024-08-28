/*-----------------------------------------------------

Plattform for Raspberry Pi Pico and other RP2040 boards
by SirSydom <com@sirsydom.de> 2021-2022

made to work with arduino-pico - "Raspberry Pi Pico Arduino core, for all RP2040 boards"
by Earl E. Philhower III https://github.com/earlephilhower/arduino-pico


RTTI must be set to enabled in the board options

Uses direct flash reading/writing.
Size ist defined by KNX_FLASH_SIZE (default 4k) - must be a multiple of 4096.
Offset in Flash is defined by KNX_FLASH_OFFSET (default 1,5MiB / 0x180000) - must be a multiple of 4096.

EEPROM Emulation from arduino-pico core (max 4k) can be use by defining USE_RP2040_EEPROM_EMULATION

A RAM-buffered Flash can be use by defining USE_RP2040_LARGE_EEPROM_EMULATION

For usage of KNX-IP you have to define either
- KNX_IP_LAN (use the arduino-pico core's w5500 lwip stack)
- KNX_IP_WIFI (use the arduino-pico core's PiPicoW lwip stack)

----------------------------------------------------*/
#ifdef ARDUINO_ARCH_RP2040
#include "rp2040_arduino_platform.h"

#include "knx/bits.h"

#include <Arduino.h>

// Pi Pico specific libs
#include <EEPROM.h>            // EEPROM emulation in flash, part of Earl E Philhowers Pi Pico Arduino support
#include <hardware/flash.h>    // from Pico SDK
#include <hardware/watchdog.h> // from Pico SDK
#include <pico/unique_id.h>    // from Pico SDK

namespace Knx
{
#ifdef USE_KNX_DMA_UART
#include <hardware/dma.h>
    // constexpr uint32_t uartDmaTransferCount = 0b1111111111;
    constexpr uint32_t uartDmaTransferCount = UINT32_MAX;
    constexpr uint8_t uartDmaBufferExp = 8u; // 2**BufferExp
    constexpr uint16_t uartDmaBufferSize = (1u << uartDmaBufferExp);
    int8_t uartDmaChannel = -1;
    volatile uint8_t __attribute__((aligned(uartDmaBufferSize))) uartDmaBuffer[uartDmaBufferSize] = {};
    volatile uint32_t uartDmaReadCount = 0;
    volatile uint16_t uartDmaRestartCount = 0;
    volatile uint32_t uartDmaWriteCount2 = 0;
    volatile uint32_t uartDmaAvail = 0;

    // Returns the number of bytes read since the DMA transfer start
    inline uint32_t uartDmaWriteCount()
    {
        uartDmaWriteCount2 = uartDmaTransferCount - dma_channel_hw_addr(uartDmaChannel)->transfer_count;
        return uartDmaWriteCount2;
    }

    // Returns the current write position in the DMA buffer
    inline uint16_t uartDmaWriteBufferPosition()
    {
        return uartDmaWriteCount() % uartDmaBufferSize;
    }

    // Returns the current read position in the DMA buffer
    inline uint16_t uartDmaReadBufferPosition()
    {
        return uartDmaReadCount % uartDmaBufferSize;
    }

    // Returns the current reading position as a pointer
    inline uint8_t* uartDmaReadAddr()
    {
        return ((uint8_t*)uartDmaBuffer + uartDmaReadBufferPosition());
    }

    // Restarts the transfer after completion.
    void __time_critical_func(uartDmaRestart)()
    {
        // println("Restart");
        uartDmaRestartCount = uartDmaWriteBufferPosition() - uartDmaReadBufferPosition();

        // if uartDmaRestartCount == 0, everything has been processed and the read count can be set to 0 again with the restart.
        if (uartDmaRestartCount == 0)
        {
            uartDmaReadCount = 0;
        }

        asm volatile("" ::: "memory");
        dma_hw->ints0 = 1u << uartDmaChannel; // clear DMA IRQ0 flag
        asm volatile("" ::: "memory");
        dma_channel_set_write_addr(uartDmaChannel, uartDmaBuffer, true);
    }
#endif

#define FLASHPTR ((uint8_t*)XIP_BASE + KNX_FLASH_OFFSET)

#ifndef USE_RP2040_EEPROM_EMULATION
    #if KNX_FLASH_SIZE % 4096
        #error "KNX_FLASH_SIZE must be multiple of 4096"
    #endif

    #if KNX_FLASH_OFFSET % 4096
        #error "KNX_FLASH_OFFSET must be multiple of 4096"
    #endif
#endif

#ifdef KNX_IP_LAN
    extern Wiznet5500lwIP KNX_NETIF;
#elif defined(KNX_IP_WIFI)
#elif defined(KNX_IP_GENERIC)

#endif

    RP2040ArduinoPlatform::RP2040ArduinoPlatform()
#if !defined(KNX_NO_DEFAULT_UART) && !defined(USE_KNX_DMA_UART)
        : ArduinoPlatform(&KNX_SERIAL)
#endif
    {
#ifdef KNX_UART_RX_PIN
        _rxPin = KNX_UART_RX_PIN;
#endif
#ifdef KNX_UART_TX_PIN
        _txPin = KNX_UART_TX_PIN;
#endif
#ifndef USE_RP2040_EEPROM_EMULATION
        _memoryType = Flash;
#endif
    }

    RP2040ArduinoPlatform::RP2040ArduinoPlatform(HardwareSerial* s)
        : ArduinoPlatform(s)
    {
#ifndef USE_RP2040_EEPROM_EMULATION
        _memoryType = Flash;
#endif
    }

    void RP2040ArduinoPlatform::knxUartPins(pin_size_t rxPin, pin_size_t txPin)
    {
        _rxPin = rxPin;
        _txPin = txPin;
    }

    bool RP2040ArduinoPlatform::overflowUart()
    {
#ifdef USE_KNX_DMA_UART
        // during dma restart
        bool ret;
        const uint32_t writeCount = uartDmaWriteCount();

        if (uartDmaRestartCount > 0)
            ret = writeCount >= (uartDmaBufferSize - uartDmaRestartCount - 1);
        else
            ret = (writeCount - uartDmaReadCount) > uartDmaBufferSize;

        // if (ret)
        // {
        //     println(uartDmaWriteBufferPosition());
        //     println(uartDmaReadBufferPosition());
        //     println(uartDmaWriteCount());
        //     println(uartDmaReadCount);
        //     println(uartDmaRestartCount);
        //     printHex("BUF: ", (const uint8_t *)uartDmaBuffer, uartDmaBufferSize);
        //     println("OVERFLOW");
        //     while (true)
        //         ;
        // }
        return ret;
#else
        SerialUART* serial = dynamic_cast<SerialUART*>(_knxSerial);
        return serial->overflow();
#endif
    }

    void RP2040ArduinoPlatform::setupUart()
    {
#ifdef USE_KNX_DMA_UART

        if (uartDmaChannel == -1)
        {
            // configure uart0
            gpio_set_function(_rxPin, GPIO_FUNC_UART);
            gpio_set_function(_txPin, GPIO_FUNC_UART);
            uart_init(KNX_DMA_UART, 19200);
            uart_set_hw_flow(KNX_DMA_UART, false, false);
            uart_set_format(KNX_DMA_UART, 8, 1, UART_PARITY_EVEN);
            uart_set_fifo_enabled(KNX_DMA_UART, false);

            // configure uart0
            uartDmaChannel = dma_claim_unused_channel(true); // get free channel for dma
            dma_channel_config dmaConfig = dma_channel_get_default_config(uartDmaChannel);
            channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_8);
            channel_config_set_read_increment(&dmaConfig, false);
            channel_config_set_write_increment(&dmaConfig, true);
            channel_config_set_high_priority(&dmaConfig, true);
            channel_config_set_ring(&dmaConfig, true, uartDmaBufferExp);
            channel_config_set_dreq(&dmaConfig, KNX_DMA_UART_DREQ);
            dma_channel_set_read_addr(uartDmaChannel, &uart_get_hw(uart0)->dr, false);
            dma_channel_set_write_addr(uartDmaChannel, uartDmaBuffer, false);
            dma_channel_set_trans_count(uartDmaChannel, uartDmaTransferCount, false);
            dma_channel_set_config(uartDmaChannel, &dmaConfig, true);
            dma_channel_set_irq1_enabled(uartDmaChannel, true);
            // irq_add_shared_handler(KNX_DMA_IRQ, uartDmaRestart, PICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY);
            irq_set_exclusive_handler(KNX_DMA_IRQ, uartDmaRestart);
            irq_set_enabled(KNX_DMA_IRQ, true);
        }

#else
        SerialUART* serial = dynamic_cast<SerialUART*>(_knxSerial);

        if (serial)
        {
            if (_rxPin != UART_PIN_NOT_DEFINED)
                serial->setRX(_rxPin);

            if (_txPin != UART_PIN_NOT_DEFINED)
                serial->setTX(_txPin);

            serial->setPollingMode();
            serial->setFIFOSize(64);
        }

        _knxSerial->begin(19200, SERIAL_8E1);

        while (!_knxSerial)
            ;

#endif
    }

#ifdef USE_KNX_DMA_UART
    int RP2040ArduinoPlatform::uartAvailable()
    {
        if (uartDmaChannel == -1)
            return 0;

        if (uartDmaRestartCount > 0)
        {
            return uartDmaRestartCount;
        }
        else
        {
            uint32_t tc = dma_channel_hw_addr(uartDmaChannel)->transfer_count;
            uartDmaAvail = tc;
            int test = uartDmaTransferCount - tc - uartDmaReadCount;
            return test;
        }
    }

    int RP2040ArduinoPlatform::readUart()
    {
        if (!uartAvailable())
            return -1;

        int ret = uartDmaReadAddr()[0];
        // print("< ");
        // println(ret, HEX);
        uartDmaReadCount++;

        if (uartDmaRestartCount > 0)
        {
            // process previouse buffer
            uartDmaRestartCount--;

            // last char, then reset read count to start at new writer position
            if (uartDmaRestartCount == 0)
                uartDmaReadCount = 0;
        }

        return ret;
    }

    size_t RP2040ArduinoPlatform::writeUart(const uint8_t data)
    {
        if (uartDmaChannel == -1)
            return 0;

        // print("> ");
        // println(data, HEX);
        while (!uart_is_writable(uart0))
            ;

        uart_putc_raw(uart0, data);
        return 1;
    }

    void RP2040ArduinoPlatform::closeUart()
    {
        if (uartDmaChannel >= 0)
        {
            dma_channel_cleanup(uartDmaChannel);
            irq_set_enabled(DMA_IRQ_0, false);
            uart_deinit(uart0);
            uartDmaChannel = -1;
            uartDmaReadCount = 0;
            uartDmaRestartCount = 0;
        }
    }
#endif

    uint32_t RP2040ArduinoPlatform::uniqueSerialNumber()
    {
        pico_unique_board_id_t id; // 64Bit unique serial number from the QSPI flash

        noInterrupts();
        rp2040.idleOtherCore();

        flash_get_unique_id(id.id); // pico_get_unique_board_id(&id);

        rp2040.resumeOtherCore();
        interrupts();

        // use lower 4 byte and convert to unit32_t
        uint32_t uid = ((uint32_t)(id.id[4]) << 24) | ((uint32_t)(id.id[5]) << 16) | ((uint32_t)(id.id[6]) << 8) | (uint32_t)(id.id[7]);

        return uid;
    }

    void RP2040ArduinoPlatform::restart()
    {
        println("restart");
        watchdog_reboot(0, 0, 0);
    }

#ifdef USE_RP2040_EEPROM_EMULATION

#pragma warning "Using EEPROM Simulation"

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION

    uint8_t* RP2040ArduinoPlatform::getEepromBuffer(uint32_t size)
    {
        if (size % 4096)
        {
            println("KNX_FLASH_SIZE must be a multiple of 4096");
            fatalError();
        }

        if (!_rambuff_initialized)
        {
            memcpy(_rambuff, FLASHPTR, KNX_FLASH_SIZE);
            _rambuff_initialized = true;
        }

        return _rambuff;
    }

    void RP2040ArduinoPlatform::commitToEeprom()
    {
        noInterrupts();
        rp2040.idleOtherCore();

        // ToDo: write block-by-block to prevent writing of untouched blocks
        if (memcmp(_rambuff, FLASHPTR, KNX_FLASH_SIZE))
        {
            flash_range_erase(KNX_FLASH_OFFSET, KNX_FLASH_SIZE);
            flash_range_program(KNX_FLASH_OFFSET, _rambuff, KNX_FLASH_SIZE);
        }

        rp2040.resumeOtherCore();
        interrupts();
    }

#else

    uint8_t* RP2040ArduinoPlatform::getEepromBuffer(uint32_t size)
    {
        if (size > 4096)
        {
            println("KNX_FLASH_SIZE to big for EEPROM emulation (max. 4kB)");
            fatalError();
        }

        uint8_t* eepromptr = EEPROM.getDataPtr();

        if (eepromptr == nullptr)
        {
            EEPROM.begin(4096);
            eepromptr = EEPROM.getDataPtr();
        }

        return eepromptr;
    }

    void RP2040ArduinoPlatform::commitToEeprom()
    {
        EEPROM.commit();
    }

#endif

#else

    size_t RP2040ArduinoPlatform::flashEraseBlockSize()
    {
        return 16; // 16 pages x 256byte/page = 4096byte
    }

    size_t RP2040ArduinoPlatform::flashPageSize()
    {
        return 256;
    }

    uint8_t* RP2040ArduinoPlatform::userFlashStart()
    {
        return (uint8_t*)XIP_BASE + KNX_FLASH_OFFSET;
    }

    size_t RP2040ArduinoPlatform::userFlashSizeEraseBlocks()
    {
        if (KNX_FLASH_SIZE <= 0)
            return 0;
        else
            return ((KNX_FLASH_SIZE - 1) / (flashPageSize() * flashEraseBlockSize())) + 1;
    }

    void RP2040ArduinoPlatform::flashErase(uint16_t eraseBlockNum)
    {
        noInterrupts();
        rp2040.idleOtherCore();

        flash_range_erase(KNX_FLASH_OFFSET + eraseBlockNum * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());

        rp2040.resumeOtherCore();
        interrupts();
    }

    void RP2040ArduinoPlatform::flashWritePage(uint16_t pageNumber, uint8_t* data)
    {
        noInterrupts();
        rp2040.idleOtherCore();

        flash_range_program(KNX_FLASH_OFFSET + pageNumber * flashPageSize(), data, flashPageSize());

        rp2040.resumeOtherCore();
        interrupts();
    }

    void RP2040ArduinoPlatform::writeBufferedEraseBlock()
    {
        if (_bufferedEraseblockNumber > -1 && _bufferedEraseblockDirty)
        {
            noInterrupts();
            rp2040.idleOtherCore();

            flash_range_erase(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), flashPageSize() * flashEraseBlockSize());
            flash_range_program(KNX_FLASH_OFFSET + _bufferedEraseblockNumber * flashPageSize() * flashEraseBlockSize(), _eraseblockBuffer, flashPageSize() * flashEraseBlockSize());

            rp2040.resumeOtherCore();
            interrupts();

            _bufferedEraseblockDirty = false;
        }
    }
#endif

#if defined(KNX_NETIF)
    uint32_t RP2040ArduinoPlatform::currentIpAddress()
    {
        return KNX_NETIF.localIP();
    }
    uint32_t RP2040ArduinoPlatform::currentSubnetMask()
    {
        return KNX_NETIF.subnetMask();
    }
    uint32_t RP2040ArduinoPlatform::currentDefaultGateway()
    {
        return KNX_NETIF.gatewayIP();
    }
    void RP2040ArduinoPlatform::macAddress(uint8_t* addr)
    {
#if defined(KNX_IP_LAN)
        addr = KNX_NETIF.getNetIf()->hwaddr;
#else
        uint8_t macaddr[6] = {0, 0, 0, 0, 0, 0};
        addr = KNX_NETIF.macAddress(macaddr);
#endif
    }

    // multicast
    void RP2040ArduinoPlatform::setupMultiCast(uint32_t addr, uint16_t port)
    {
        mcastaddr = IPAddress(htonl(addr));
        _port = port;
        uint8_t result = _udp.beginMulticast(mcastaddr, port);
        (void)result;

#ifdef KNX_IP_GENERIC
        // if(!_unicast_socket_setup)
        //     _unicast_socket_setup = UDP_UNICAST.begin(3671);
#endif

        // print("Setup Mcast addr: ");
        // print(mcastaddr.toString().c_str());
        // print(" on port: ");
        // print(port);
        // print(" result ");
        // println(result);
    }

    void RP2040ArduinoPlatform::closeMultiCast()
    {
        _udp.stop();
    }

    bool RP2040ArduinoPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
    {
        // printHex("<- ",buffer, len);

        // ToDo: check if Ethernet is able to receive, return false if not
        _udp.beginPacket(mcastaddr, _port);
        _udp.write(buffer, len);
        _udp.endPacket();
        return true;
    }

    int RP2040ArduinoPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port)
    {
        int len = _udp.parsePacket();

        if (len == 0)
            return 0;

        if (len > maxLen)
        {
            println("Unexpected UDP data packet length - drop packet");

            for (size_t i = 0; i < len; i++)
                _udp.read();

            return 0;
        }

        _udp.read(buffer, len);
        _remoteIP = _udp.remoteIP();
        _remotePort = _udp.remotePort();
        src_addr = htonl(_remoteIP);
        src_port = _remotePort;

        // print("Remote IP: ");
        // print(_udp.remoteIP().toString().c_str());
        // printHex("-> ", buffer, len);

        return len;
    }

    // unicast
    bool RP2040ArduinoPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
    {
        IPAddress ucastaddr(htonl(addr));

        if (!addr)
            ucastaddr = _remoteIP;

        if (!port)
            port = _remotePort;

        // print("sendBytesUniCast to:");
        // println(ucastaddr.toString().c_str());

#ifdef KNX_IP_GENERIC

        if (!_unicast_socket_setup)
            _unicast_socket_setup = UDP_UNICAST.begin(3671);

#endif

        if (UDP_UNICAST.beginPacket(ucastaddr, port) == 1)
        {
            UDP_UNICAST.write(buffer, len);

            if (UDP_UNICAST.endPacket() == 0)
                println("sendBytesUniCast endPacket fail");
        }
        else
            println("sendBytesUniCast beginPacket fail");

        return true;
    }
#endif
}
#endif