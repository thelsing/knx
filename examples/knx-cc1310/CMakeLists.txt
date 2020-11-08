##
## KNX for CC1310
##

cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain-arm-none-eabi.cmake")
endif()    

project(knx-cc1310 C CXX ASM)

# Generate a "compile_commands.json" file containing the exact compiler calls for all translation units of the project in machine-readable form.
# Useful for C/CPP extension of VS code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use <Package>_ROOT variables for locating in find_path, find_package, etc.
cmake_policy(SET CMP0074 NEW)

# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
message("CMAKE_MODULE_PATH:  ${CMAKE_MODULE_PATH}")

# Configure Toolchain for CC13xx
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/CC1310_LAUNCHXL_NoRTOS.lds")
include(cc13xx)

set(SimpleLinkCC13X0SDKnortos_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/coresdk_cc13xx_cc26xx")
message("SimpleLinkCC13X0SDKnortos_ROOT: ${SimpleLinkCC13X0SDKnortos_ROOT}")

# Find dependencies
find_package(SimpleLinkCC13X0SDKnortos REQUIRED)

#
# Target
#

set(${PROJECT_NAME}_SOURCES
    ../../src/knx/address_table_object.cpp
    ../../src/knx/address_table_object.h
    ../../src/knx/aes.c
    ../../src/knx/aes.h
    ../../src/knx/aes.hpp
    ../../src/knx/apdu.cpp
    ../../src/knx/apdu.h
    ../../src/knx/application_layer.cpp
    ../../src/knx/application_layer.h
    ../../src/knx/application_program_object.cpp
    ../../src/knx/application_program_object.h
    ../../src/knx/association_table_object.cpp
    ../../src/knx/association_table_object.h
    ../../src/knx/bau.cpp
    ../../src/knx/bau.h
    ../../src/knx/bau07B0.cpp
    ../../src/knx/bau07B0.h
    ../../src/knx/bau091A.cpp
    ../../src/knx/bau091A.h
    ../../src/knx/bau27B0.cpp
    ../../src/knx/bau27B0.h
    ../../src/knx/bau2920.cpp
    ../../src/knx/bau2920.h
    ../../src/knx/bau57B0.cpp
    ../../src/knx/bau57B0.h
    ../../src/knx/bau_systemB.cpp
    ../../src/knx/bau_systemB.h
    ../../src/knx/bau_systemB_device.cpp
    ../../src/knx/bau_systemB_device.h
    ../../src/knx/bau_systemB_coupler.cpp
    ../../src/knx/bau_systemB_coupler.h
    ../../src/knx/bits.cpp
    ../../src/knx/bits.h
    ../../src/knx/callback_property.h
    ../../src/knx/cemi_frame.cpp
    ../../src/knx/cemi_frame.h
    ../../src/knx/cemi_server.cpp
    ../../src/knx/cemi_server.h
    ../../src/knx/cemi_server_object.cpp
    ../../src/knx/cemi_server_object.h
    ../../src/knx/config.h
    ../../src/knx/data_link_layer.cpp
    ../../src/knx/data_link_layer.h
    ../../src/knx/data_property.cpp
    ../../src/knx/data_property.h
    ../../src/knx/device_object.cpp
    ../../src/knx/device_object.h
    ../../src/knx/dpt.cpp
    ../../src/knx/dpt.h
    ../../src/knx/dptconvert.cpp
    ../../src/knx/dptconvert.h
    ../../src/knx/function_property.h
    ../../src/knx/group_object.cpp
    ../../src/knx/group_object.h
    ../../src/knx/group_object_table_object.cpp
    ../../src/knx/group_object_table_object.h
    ../../src/knx/interface_object.cpp
    ../../src/knx/interface_object.h
    ../../src/knx/ip_data_link_layer.cpp
    ../../src/knx/ip_data_link_layer.h
    ../../src/knx/ip_parameter_object.cpp
    ../../src/knx/ip_parameter_object.h
    ../../src/knx/knx_ip_device_information_dib.cpp
    ../../src/knx/knx_ip_device_information_dib.h
    ../../src/knx/knx_ip_dib.cpp
    ../../src/knx/knx_ip_dib.h
    ../../src/knx/knx_ip_frame.cpp
    ../../src/knx/knx_ip_frame.h
    ../../src/knx/knx_ip_routing_indication.cpp
    ../../src/knx/knx_ip_routing_indication.h
    ../../src/knx/knx_ip_search_request.cpp
    ../../src/knx/knx_ip_search_request.h
    ../../src/knx/knx_ip_search_response.cpp
    ../../src/knx/knx_ip_search_response.h
    ../../src/knx/knx_ip_supported_service_dib.cpp
    ../../src/knx/knx_ip_supported_service_dib.h
    ../../src/knx/ip_host_protocol_address_information.cpp
    ../../src/knx/ip_host_protocol_address_information.h
    ../../src/knx/knx_types.h
    ../../src/knx/knx_value.cpp
    ../../src/knx/knx_value.h
    ../../src/knx/memory.cpp
    ../../src/knx/memory.h
    ../../src/knx/network_layer.cpp
    ../../src/knx/network_layer.h
    ../../src/knx/network_layer_coupler.cpp
    ../../src/knx/network_layer_coupler.h
    ../../src/knx/network_layer_device.cpp
    ../../src/knx/network_layer_device.h
    ../../src/knx/network_layer_entity.cpp
    ../../src/knx/network_layer_entity.h
    ../../src/knx/npdu.cpp
    ../../src/knx/npdu.h
    ../../src/knx/platform.cpp
    ../../src/knx/platform.h
    ../../src/knx/property.cpp
    ../../src/knx/property.h
    ../../src/knx/rf_data_link_layer.cpp
    ../../src/knx/rf_data_link_layer.h
    ../../src/knx/rf_medium_object.cpp
    ../../src/knx/rf_medium_object.h
    ../../src/knx/rf_physical_layer_cc1310.cpp
    ../../src/knx/rf_physical_layer_cc1310.h
    ../../src/knx/rf_physical_layer.h
    ../../src/knx/router_object.cpp
    ../../src/knx/router_object.h
    ../../src/knx/secure_application_layer.cpp
    ../../src/knx/secure_application_layer.h
    ../../src/knx/security_interface_object.cpp
    ../../src/knx/security_interface_object.h
    ../../src/knx/simple_map.h
    ../../src/knx/save_restore.h
    ../../src/knx/table_object.cpp
    ../../src/knx/table_object.h
    ../../src/knx/tpdu.cpp
    ../../src/knx/tpdu.h
    ../../src/knx/tpuart_data_link_layer.cpp
    ../../src/knx/tpuart_data_link_layer.h
    ../../src/knx/transport_layer.cpp
    ../../src/knx/transport_layer.h
    ../../src/knx/usb_tunnel_interface.cpp
    ../../src/knx/usb_tunnel_interface.h
    ../../src/knx_facade.cpp
    ../../src/knx_facade.h
    ../../src/cc1310_platform.cpp
    ../../src/knx_facade.cpp
    ./RTT/SEGGER_RTT_Conf.h
    ./RTT/SEGGER_RTT_printf.c
    ./RTT/SEGGER_RTT.c
    ./RTT/SEGGER_RTT.h
    ./RTT/SEGGER_RTT_ASM_ARMv7M.S
    ./smartrf_settings/smartrf_settings.c
    ./smartrf_settings/smartrf_settings.h
    ./CC1310_LAUNCHXL_fxns.c
    ./CC1310_LAUNCHXL.c
    ./CC1310_LAUNCHXL.h
    ./knx_wrapper.cpp
    ./knx_wrapper.h
    ./ccfg.c
    ./main_nortos.c
    ./startup_cc13xx_cc26xx_gcc.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/knx
    ${CMAKE_CURRENT_SOURCE_DIR}/RTT
    ${SimpleLinkCC13X0SDK_INCLUDE_DIRS}
)

add_definitions(-DMASK_VERSION=0x27B0 -Wno-unknown-pragmas)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    -Wl,--start-group
    ${SimpleLinkCC13X0SDK_drivers_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_dpl_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_driverlib_LIBRARY}
    ${SimpleLinkCC13X0SDK_rf_singleMode_cc13x0_LIBRARY}
    -Wl,--end-group
    gcc
    m
    nosys
    c
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DDeviceFamily_CC13X0 -DRF_SINGLEMODE -DKNX_FLASH_SIZE=2048)

