cmake_minimum_required(VERSION 3.16)
project(knx-linux VERSION 1.5)
set(LIBRARIES_FROM_REFERENCES "")

set(SOURCES fdsk.cpp fdsk.h main.cpp)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wno-unknown-pragmas -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-unknown-pragmas -g -O0")

add_executable(knx-linux-tp ${SOURCES})
target_link_libraries(knx-linux-tp knx "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-tp PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-tp PUBLIC -DMASK_VERSION=0x07B0)

add_executable(knx-linux-rf ${SOURCES})
target_link_libraries(knx-linux-rf knx "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-rf PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-rf PUBLIC -DMASK_VERSION=0x27B0)

add_executable(knx-linux-ip ${SOURCES})
target_link_libraries(knx-linux-ip knx "${LIBRARIES_FROM_REFERENCES}")
set_property(TARGET knx-linux-ip PROPERTY CXX_STANDARD 11)
target_compile_definitions(knx-linux-ip PUBLIC -DMASK_VERSION=0x57B0)
