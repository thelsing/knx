cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# require for knx components
add_definitions(
  -Wno-unknown-pragmas
  -DMASK_VERSION=0x07B0
  -DKNX_NO_AUTOMATIC_GLOBAL_INSTANCE
  -DKNX_FLASH_SIZE=4096
  #-DKNX_NO_PRINT
  #-Wno-stringop-truncation
)

project(knx-demo-diy-idf)