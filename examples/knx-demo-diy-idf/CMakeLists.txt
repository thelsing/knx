cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# require for knx components
add_definitions(
  -Wno-unknown-pragmas
  -DMASK_VERSION=0x07B0
  -DKNX_NO_AUTOMATIC_GLOBAL_INSTANCE
  -DKNX_FLASH_SIZE=4096
  -DKNX_NO_PRINT
  -Wno-stringop-truncation ## remove warning strncpy((char *)sta_wifi_config.sta.ssid, config_values.ssid, sizeof(sta_wifi_config.sta.ssid)); in  tuya.h
)

project(knx-demo-diy-idf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# idf_component_register(
#     SRCS "main.c"
#     INCLUDE_DIRS ".." "../../src"
#     REQUIRES freertos nvs_flash esp_wifi esp_system
# ) 