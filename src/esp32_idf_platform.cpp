#ifndef ARDUINO
#ifdef ESP_PLATFORM
// esp32_idf_platform.cpp
#include <esp_system.h>
#include <esp_mac.h>
#include "esp32_idf_platform.h"
#include "esp_log.h"
#include "knx/bits.h"
#include "nvs.h"
#include <esp_timer.h>

static const char* KTAG = "KNX_LIB";

Esp32IdfPlatform::Esp32IdfPlatform(uart_port_t uart_num)
    : _uart_num(uart_num)
{
    // Set the memory type to use our NVS-based EEPROM emulation
    _memoryType = Eeprom;
}

Esp32IdfPlatform::~Esp32IdfPlatform()
{
    if (_sock != -1)
    {
        closeMultiCast();
    }
    if (_uart_installed)
    {
        closeUart();
    }
    if (_eeprom_buffer)
    {
        free(_eeprom_buffer);
    }
    if (_nvs_handle)
    {
        nvs_close(_nvs_handle);
    }
}

void Esp32IdfPlatform::knxUartPins(int8_t rxPin, int8_t txPin)
{
    _rxPin = rxPin;
    _txPin = txPin;
}

void Esp32IdfPlatform::setNetif(esp_netif_t* netif)
{
    _netif = netif;
}

void Esp32IdfPlatform::fatalError()
{
    ESP_LOGE(KTAG, "FATAL ERROR. System halted.");
    // Loop forever to halt the system
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ESP specific uart handling with pins
void Esp32IdfPlatform::setupUart()
{
    if (_uart_installed)
        return;
    uart_config_t uart_config;
    memset(&uart_config, 0, sizeof(uart_config));
    uart_config.baud_rate = 19200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_EVEN;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    
    ESP_ERROR_CHECK(uart_driver_install(_uart_num, 256 * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(_uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(_uart_num, _txPin, _rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    _uart_installed = true;
}

void Esp32IdfPlatform::closeUart()
{
    if (!_uart_installed)
        return;
    uart_driver_delete(_uart_num);
    _uart_installed = false;
}

int Esp32IdfPlatform::uartAvailable()
{
    if (!_uart_installed)
        return 0;
    size_t length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(_uart_num, &length));
    return length;
}

size_t Esp32IdfPlatform::writeUart(const uint8_t data)
{
    if (!_uart_installed)
        return 0;
    return uart_write_bytes(_uart_num, &data, 1);
}

size_t Esp32IdfPlatform::writeUart(const uint8_t* buffer, size_t size)
{
    if (!_uart_installed)
        return 0;
    return uart_write_bytes(_uart_num, buffer, size);
}

int Esp32IdfPlatform::readUart()
{
    if (!_uart_installed)
        return -1;
    uint8_t data;
    if (uart_read_bytes(_uart_num, &data, 1, pdMS_TO_TICKS(20)) > 0)
    {
        return data;
    }
    return -1;
}

size_t Esp32IdfPlatform::readBytesUart(uint8_t* buffer, size_t length)
{
    if (!_uart_installed)
        return 0;
    return uart_read_bytes(_uart_num, buffer, length, pdMS_TO_TICKS(100));
}

void Esp32IdfPlatform::flushUart()
{
    if (!_uart_installed)
        return;
    ESP_ERROR_CHECK(uart_flush(_uart_num));
}

uint32_t Esp32IdfPlatform::currentIpAddress()
{
    if (!_netif)
        return 0;
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(_netif, &ip_info);
    return ip_info.ip.addr;
}

uint32_t Esp32IdfPlatform::currentSubnetMask()
{
    if (!_netif)
        return 0;
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(_netif, &ip_info);
    return ip_info.netmask.addr;
}

uint32_t Esp32IdfPlatform::currentDefaultGateway()
{
    if (!_netif)
        return 0;
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(_netif, &ip_info);
    return ip_info.gw.addr;
}

void Esp32IdfPlatform::macAddress(uint8_t* addr)
{
    if (!_netif)
        return;
    esp_netif_get_mac(_netif, addr);
}

uint32_t Esp32IdfPlatform::uniqueSerialNumber()
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    uint64_t chipid = 0;
    for (int i = 0; i < 6; i++)
    {
        chipid |= ((uint64_t)mac[i] << (i * 8));
    }
    uint32_t upperId = (chipid >> 32) & 0xFFFFFFFF;
    uint32_t lowerId = (chipid & 0xFFFFFFFF);
    return (upperId ^ lowerId);
}

void Esp32IdfPlatform::restart()
{
    ESP_LOGI(KTAG, "Restarting system...");
    esp_restart();
}

void Esp32IdfPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    _multicast_addr = addr;
    _multicast_port = port;

    _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (_sock < 0)
    {
        ESP_LOGE(KTAG, "Failed to create socket. Errno: %d", errno);
        return;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(_sock, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in)) < 0)
    {
        ESP_LOGE(KTAG, "Failed to bind socket. Errno: %d", errno);
        close(_sock);
        _sock = -1;
        return;
    }

    struct ip_mreq imreq;
    memset(&imreq, 0, sizeof(imreq));
    imreq.imr_interface.s_addr = IPADDR_ANY;
    imreq.imr_multiaddr.s_addr = addr;
    if (setsockopt(_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(struct ip_mreq)) < 0)
    {
        ESP_LOGE(KTAG, "Failed to join multicast group. Errno: %d", errno);
        close(_sock);
        _sock = -1;
        return;
    }

    ESP_LOGI(KTAG, "Successfully joined multicast group on port %d", port);
}

void Esp32IdfPlatform::closeMultiCast()
{
    if (_sock != -1)
    {
        close(_sock);
        _sock = -1;
    }
}

bool Esp32IdfPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
{
    if (_sock < 0)
        return false;

    struct sockaddr_in dest_addr = {};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(_multicast_port);
    dest_addr.sin_addr.s_addr = _multicast_addr;

    int sent_len = sendto(_sock, buffer, len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_len < 0)
    {
        ESP_LOGE(KTAG, "sendBytesMultiCast failed. Errno: %d", errno);
        return false;
    }
    return sent_len == len;
}

int Esp32IdfPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen, uint32_t& src_addr, uint16_t& src_port)
{
    if (_sock < 0)
        return 0;

    socklen_t socklen = sizeof(_remote_addr);
    int len = recvfrom(_sock, buffer, maxLen, 0, (struct sockaddr*)&_remote_addr, &socklen);

    if (len <= 0)
    {
        return 0; // No data or error
    }

    src_addr = _remote_addr.sin_addr.s_addr;
    src_port = ntohs(_remote_addr.sin_port);

    return len;
}

bool Esp32IdfPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    if (_sock < 0)
        return false;

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;

    if (addr == 0)
    { // If address is 0, use the address from the last received packet
        dest_addr.sin_addr.s_addr = _remote_addr.sin_addr.s_addr;
    }
    else
    {
        dest_addr.sin_addr.s_addr = addr;
    }

    if (port == 0)
    { // If port is 0, use the port from the last received packet
        dest_addr.sin_port = _remote_addr.sin_port;
    }
    else
    {
        dest_addr.sin_port = htons(port);
    }

    if (sendto(_sock, buffer, len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0)
    {
        ESP_LOGE(KTAG, "sendBytesUniCast failed. Errno: %d", errno);
        return false;
    }

    return true;
}

uint8_t* Esp32IdfPlatform::getEepromBuffer(uint32_t size)
{
    if (_eeprom_buffer && _eeprom_size == size)
    {
        return _eeprom_buffer;
    }

    if (_eeprom_buffer)
    {
        free(_eeprom_buffer);
        _eeprom_buffer = nullptr;
    }

    _eeprom_size = size;
    _eeprom_buffer = (uint8_t*)malloc(size);
    if (!_eeprom_buffer)
    {
        ESP_LOGE(KTAG, "Failed to allocate EEPROM buffer");
        fatalError();
        return nullptr;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open(_nvs_namespace, NVS_READWRITE, &_nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(KTAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        free(_eeprom_buffer);
        _eeprom_buffer = nullptr;
        fatalError();
        return nullptr;
    }

    size_t required_size = size;
    err = nvs_get_blob(_nvs_handle, _nvs_key, _eeprom_buffer, &required_size);
    if (err != ESP_OK || required_size != size)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGI(KTAG, "No previous EEPROM data found in NVS. Initializing fresh buffer.");
        }
        else
        {
            ESP_LOGW(KTAG, "NVS get blob failed (%s) or size mismatch. Initializing fresh buffer.", esp_err_to_name(err));
        }
        memset(_eeprom_buffer, 0xFF, size);
    }
    else
    {
        ESP_LOGI(KTAG, "Successfully loaded %d bytes from NVS into EEPROM buffer.", required_size);
    }

    return _eeprom_buffer;
}

void Esp32IdfPlatform::commitToEeprom()
{
    if (!_eeprom_buffer || !_nvs_handle)
    {
        ESP_LOGE(KTAG, "EEPROM not initialized, cannot commit.");
        return;
    }

    esp_err_t err = nvs_set_blob(_nvs_handle, _nvs_key, _eeprom_buffer, _eeprom_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(KTAG, "Failed to set NVS blob: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_commit(_nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(KTAG, "Failed to commit NVS: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(KTAG, "Committed %" PRIu32 " bytes to NVS.", _eeprom_size);
    }
}

uint32_t millis()
{
    // esp_timer_get_time() returns microseconds, so we divide by 1000 for milliseconds.
    // Cast to uint32_t to match the Arduino function signature.
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// Internal wrapper function to bridge Arduino-style ISR to ESP-IDF
static void IRAM_ATTR isr_wrapper(void* arg)
{
    IsrFuncPtr fn = (IsrFuncPtr)arg;
    fn();  // call the original ISR
}

// Implement attachInterrupt arduino like in ESP IDF
void attachInterrupt(uint32_t pin, IsrFuncPtr callback, uint32_t mode)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = (gpio_int_type_t)mode
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
       // Add ISR using the wrapper and pass original function as argument
    ESP_ERROR_CHECK(gpio_isr_handler_add((gpio_num_t)pin, isr_wrapper, (void*)callback));
}

#endif // ESP_PLATFORM
#endif // !ARDUINO