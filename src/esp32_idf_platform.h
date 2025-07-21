#ifndef ARDUINO
#ifdef ESP_PLATFORM
// esp_idf_platform.h
#pragma once

#include "driver/uart.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "knx/platform.h"// Include the provided base class

class Esp32IdfPlatform : public Platform
{
  public:
    Esp32IdfPlatform(uart_port_t uart_num = UART_NUM_1);
    ~Esp32IdfPlatform();

    // uart
    void knxUartPins(int8_t rxPin, int8_t txPin);

    // Call this after WiFi/Ethernet has started and received an IP.
    void setNetif(esp_netif_t* netif);

    // --- Overridden Virtual Functions ---

    // ip stuff
    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // unique serial number
    uint32_t uniqueSerialNumber() override;

    // basic stuff (pure virtual in base)
    void restart() override;
    void fatalError() override;

    // multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
    int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port) override;

    // unicast
    bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;

    // UART
    void setupUart() override;
    void closeUart() override;
    int uartAvailable() override;
    size_t writeUart(const uint8_t data) override;
    size_t writeUart(const uint8_t* buffer, size_t size) override;
    int readUart() override;
    size_t readBytesUart(uint8_t* buffer, size_t length) override;
    void flushUart() override;

    // Memory (EEPROM emulation via NVS)
    // We override these two functions to provide the low-level storage mechanism.
    // The base Platform class will use them when _memoryType is Eeprom.
    uint8_t* getEepromBuffer(uint32_t size) override;
    void commitToEeprom() override;

  private:
    // Network
    esp_netif_t* _netif = nullptr;
    int _sock = -1;
    struct sockaddr_in _remote_addr;
    uint32_t _multicast_addr = 0;
    uint16_t _multicast_port = 0;

    // UART
    uart_port_t _uart_num;
    int8_t _rxPin = -1;
    int8_t _txPin = -1;
    bool _uart_installed = false;

    // NVS (for EEPROM emulation)
    nvs_handle_t _nvs_handle;
    uint8_t* _eeprom_buffer = nullptr;
    uint32_t _eeprom_size = 0;
    const char* _nvs_namespace = "eeprom";
    const char* _nvs_key = "eeprom";
};
#endif
#endif