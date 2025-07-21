idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."        
    REQUIRES
        knx
        esp_timer
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        mdns
)