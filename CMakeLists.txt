cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.16)
project(knx VERSION 1.5)

add_subdirectory(src)
add_subdirectory(examples/knx-linux)
add_subdirectory(examples/knx-linux-coupler)
#add_subdirectory(examples/knxPython)
