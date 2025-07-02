#ifdef ESP_PLATFORM
#include "esp32_idf_platform.h"
#include <cstring>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_event.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <lwip/ip_addr.h>
#include <driver/uart.h>

#define KNX_IDF_UART_NUM UART_NUM_1
#define KNX_IDF_UART_BAUD 19200
#define KNX_IDF_UART_TX_BUF_SIZE 256
#define KNX_IDF_UART_RX_BUF_SIZE 256

Esp32IdfPlatform::Esp32IdfPlatform()
    : ArduinoPlatform(nullptr) // No HardwareSerial in IDF
{
    // Optionally initialize NVS, WiFi, etc. here
}

void Esp32IdfPlatform::knxUartPins(int8_t rxPin, int8_t txPin) {
    _rxPin = rxPin;
    _txPin = txPin;
}

void Esp32IdfPlatform::setupUart() {
    uart_config_t uart_config = {
        .baud_rate = KNX_IDF_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(KNX_IDF_UART_NUM, &uart_config);
    uart_set_pin(KNX_IDF_UART_NUM, _txPin, _rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(KNX_IDF_UART_NUM, KNX_IDF_UART_RX_BUF_SIZE, KNX_IDF_UART_TX_BUF_SIZE, 0, NULL, 0);
}

uint32_t Esp32IdfPlatform::currentIpAddress() {
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        return ip_info.ip.addr;
    }
    return 0;
}

uint32_t Esp32IdfPlatform::currentSubnetMask() {
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        return ip_info.netmask.addr;
    }
    return 0;
}

uint32_t Esp32IdfPlatform::currentDefaultGateway() {
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        return ip_info.gw.addr;
    }
    return 0;
}

void Esp32IdfPlatform::macAddress(uint8_t* addr) {
    esp_read_mac(addr, ESP_MAC_WIFI_STA);
}

uint32_t Esp32IdfPlatform::uniqueSerialNumber() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    uint32_t upper = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3];
    uint32_t lower = (mac[4] << 8) | mac[5];
    return upper ^ lower;
}

void Esp32IdfPlatform::restart() {
    esp_restart();
}

void Esp32IdfPlatform::setupMultiCast(uint32_t addr, uint16_t port) {
    if (_udpSock >= 0) close(_udpSock);
    _udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in mcast_addr = {};
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = htonl(addr);
    mcast_addr.sin_port = htons(port);
    // Set socket options for multicast as needed
    // ...
}

void Esp32IdfPlatform::closeMultiCast() {
    if (_udpSock >= 0) {
        close(_udpSock);
        _udpSock = -1;
    }
}

bool Esp32IdfPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len) {
    // Implement sending to multicast group
    // ...
    return true;
}

int Esp32IdfPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port) {
    // Implement reading from multicast socket
    // ...
    return 0;
}

bool Esp32IdfPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) {
    // Implement sending to unicast address
    // ...
    return true;
}

uint8_t* Esp32IdfPlatform::getEepromBuffer(uint32_t size) {
    // Use NVS or other ESP-IDF storage
    // ...
    return nullptr;
}

void Esp32IdfPlatform::commitToEeprom() {
    // Commit NVS or other storage
    // ...
}

#endif 