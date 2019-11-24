message(STATUS "KNX Stack")

include_directories("${CMAKE_CURRENT_LIST_DIR}/src")

file(GLOB KNX_STACK
    knx-stack/src/knx/address_table_object.cpp
    knx-stack/src/knx/apdu.cpp
    knx-stack/src/knx/application_layer.cpp
    knx-stack/src/knx/application_program_object.cpp
    knx-stack/src/knx/association_table_object.cpp
    knx-stack/src/knx/bau07B0.cpp
    knx-stack/src/knx/bau_systemB.cpp
    knx-stack/src/knx/bau.cpp
    knx-stack/src/knx/bits.cpp
    knx-stack/src/knx/cemi_frame.cpp
    knx-stack/src/knx/data_link_layer.cpp
    knx-stack/src/knx/datapoint_types.cpp
    knx-stack/src/knx/device_object.cpp
    knx-stack/src/knx/dpt.cpp
    knx-stack/src/knx/dptconvert.cpp
    knx-stack/src/knx/group_object.cpp
    knx-stack/src/knx/group_object_table_object.cpp
    knx-stack/src/knx/interface_object.cpp
    knx-stack/src/knx/ip_data_link_layer.cpp
    knx-stack/src/knx/ip_parameter_object.cpp
    knx-stack/src/knx/knx_value.cpp
    knx-stack/src/knx/memory.cpp
    knx-stack/src/knx/network_layer.cpp
    knx-stack/src/knx/npdu.cpp
    knx-stack/src/knx/platform.cpp
    knx-stack/src/knx/table_object.cpp
    knx-stack/src/knx/tpdu.cpp
    knx-stack/src/knx/tpuart_data_link_layer.cpp
    knx-stack/src/knx/transport_layer.cpp
    knx-stack/src/knx_facade.cpp
    knx-stack/src/stm32_platform.cpp
    knx-stack/src/ring_buffer.c
    knx-stack/src/stdio_print.cpp
)

# To ignore pragma regions in KNX stack code
add_definitions("-Wno-unknown-pragmas")
add_definitions("-Wno-switch")
add_definitions("-Wno-comment")
