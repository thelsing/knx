cmake_minimum_required(VERSION 2.7)
project(knx-linux)
set(LIBRARIES_FROM_REFERENCES "")

set(SOURCES
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
        ../../src/knx/rf_physical_layer_cc1101.cpp
        ../../src/knx/rf_physical_layer_cc1101.h
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
        ../../src/linux_platform.cpp
        ../../src/linux_platform.h
        fdsk.cpp
        fdsk.h
	main.cpp)

include_directories(../../src)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wno-unknown-pragmas -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-unknown-pragmas -g -O0")

add_executable(knx-linux-tp ${SOURCES})
target_link_libraries(knx-linux-tp  "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-tp PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-tp PUBLIC -DMASK_VERSION=0x07B0)

add_executable(knx-linux-rf ${SOURCES})
target_link_libraries(knx-linux-rf  "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-rf PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-rf PUBLIC -DMASK_VERSION=0x27B0)

add_executable(knx-linux-ip ${SOURCES})
target_link_libraries(knx-linux-ip  "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-ip PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-ip PUBLIC -DMASK_VERSION=0x57B0)
