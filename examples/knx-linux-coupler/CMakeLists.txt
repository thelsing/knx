cmake_minimum_required(VERSION 3.16)
project(knx-linux-coupler VERSION 1.5)
set(LIBRARIES_FROM_REFERENCES "")

set(SOURCES
        fdsk.cpp
        fdsk.h
	main.cpp)

include_directories(../../src)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wno-unknown-pragmas -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-unknown-pragmas -g -O0")

add_executable(knx-linux-iptp ${SOURCES})
target_link_libraries(knx-linux-iptp knx "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-iptp PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-iptp PUBLIC -DMASK_VERSION=0x091A -DKNX_TUNNELING)

add_executable(knx-linux-tprf ${SOURCES})
target_link_libraries(knx-linux-tprf knx "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-tprf PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-tprf PUBLIC -DMASK_VERSION=0x2920)
