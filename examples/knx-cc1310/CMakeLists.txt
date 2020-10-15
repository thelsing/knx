##
## Uatecho Example
##

project(knx-cc1310 C CXX ASM)
cmake_minimum_required(VERSION 3.6)

message("CMAKE_MODULE_PATH:  ${CMAKE_MODULE_PATH}")

# Configure Toolchain for CC13xx
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/CC1310_LAUNCHXL_NoRTOS.lds")
include(cc13xx)

# Find dependencies
find_package(SimpleLinkCC13X0SDK_nortos REQUIRED)

#
# Target
#

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/cc1310_platform.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/knx_facade.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/knx/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/RTT/*.c*"
    "${CMAKE_CURRENT_SOURCE_DIR}/RTT/*.S"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c*"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/knx"
    "${CMAKE_CURRENT_SOURCE_DIR}/RTT"
    ${SimpleLinkCC13X0SDK_INCLUDE_DIRS}
)

add_definitions(-DMASK_VERSION=0x07B0 -Wno-unknown-pragmas)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    ${SimpleLinkCC13X0SDK_drivers_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_dpl_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_driverlib_LIBRARY}
    gcc
    m
    nosys
    c
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DDeviceFamily_CC13X0 )

