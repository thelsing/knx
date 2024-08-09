cmake_policy(SET CMP0048 NEW)
project(knx VERSION 1.4)
cmake_minimum_required(VERSION 3.16)

add_subdirectory(examples/knx-linux)
add_subdirectory(examples/knx-linux-coupler)
add_subdirectory(examples/knxPython)
