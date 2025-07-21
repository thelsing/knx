#include "esp32_idf_platform.h"
#include "knx_facade.h"
#include "knx/bau07B0.h"
#include "knx/group_object.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>
#include <stdlib.h>
#include <limits.h>

#define WIFI_SSID      "your_ssid"
#define WIFI_PASS      "your_password"
#define MASK_VERSION   0x07B0

static const char *TAG = "knx-demo";

// --- KNX Group Object Shortcuts ---
#define goCurrent knx.getGroupObject(1)
#define goMax     knx.getGroupObject(2)
#define goMin     knx.getGroupObject(3)
#define goReset   knx.getGroupObject(4)

// --- Global Variables ---
float currentValue = 0;
float maxValue = 0;
float minValue = RAND_MAX;
int64_t lastsend = 0;

// --- KNX Stack Instance (migrated pattern) ---
Esp32IdfPlatform knxPlatform(UART_NUM_1); // Use UART_NUM_1, change if needed
Bau07B0 knxBau(knxPlatform);
KnxFacade<Esp32IdfPlatform, Bau07B0> knx(knxBau);

// --- WiFi event handler ---
static esp_netif_t* s_wifi_netif = nullptr;
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying connection to the WiFi AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        if (s_wifi_netif) {
            knxPlatform.setNetif(s_wifi_netif);
        }
    }
}

// --- Button ISR (simulate with a function call or GPIO interrupt in real use) ---
void myButtonPressed() {
    static int64_t lastpressed = 0;
    int64_t now = esp_timer_get_time() / 1000; // ms
    if (now - lastpressed > 200) {
        knx.toggleProgMode();
        lastpressed = now;
    }
}

// --- KNX Reset Callback ---
void resetCallback(GroupObject& go) {
    if (go.value()) {
        maxValue = 0;
        minValue = 10000;
    }
}

// --- Simulate temperature measurement ---
void measureTemp() {
    int64_t now = esp_timer_get_time() / 1000; // ms
    if ((now - lastsend) < 2000)
        return;

    lastsend = now;
    int r = rand();
    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100 * 100;

    goCurrent.value(currentValue);

    if (currentValue > maxValue) {
        maxValue = currentValue;
        goMax.value(maxValue);
    }
    if (currentValue < minValue) {
        minValue = currentValue;
        goMin.value(minValue);
    }
}

extern "C" void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP and WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_wifi_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialization finished.");

    // Set UART pins (example: RX=16, TX=17)
    knxPlatform.knxUartPins(16, 17);
    knxPlatform.setupUart();

    // Set button ISR
    knx.setButtonISRFunction(myButtonPressed);

    // Read KNX memory (address table, etc.)
    knx.readMemory();

    // Register group object callbacks and types if configured
    if (knx.configured()) {
        goReset.callback(resetCallback);
        goReset.dataPointType(DPT_Trigger);
        goCurrent.dataPointType(DPT_Value_Temp);
        goMin.dataPointType(DPT_Value_Temp);
        goMax.dataPointType(DPT_Value_Temp);

        ESP_LOGI(TAG, "Timeout: %d", knx.paramByte(0));
        ESP_LOGI(TAG, "Cyclic send: %d", knx.paramByte(1));
        ESP_LOGI(TAG, "Min/Max send: %d", knx.paramByte(2));
        ESP_LOGI(TAG, "Send on change: %d", knx.paramByte(3));
        ESP_LOGI(TAG, "Alignment: %d", knx.paramByte(4));
    }

    // Start KNX stack
    knx.start();

    // Main loop
    while (1) {
        knx.loop();
        if (knx.configured()) {
            measureTemp();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
} 