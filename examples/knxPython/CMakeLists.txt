cmake_minimum_required(VERSION 3.16)
project(knxPython VERSION 1.5)

add_subdirectory(pybind11)
pybind11_add_module(knxPython
	knxmodule.cpp
	)
include_directories(src pybind11/include)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0")
#set(outdir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
#set_target_properties(knx PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${outdir})
set_target_properties(knxPython PROPERTIES OUTPUT_NAME knx)
set_property(TARGET knxPython PROPERTY CXX_STANDARD 11)
#target_compile_definitions(knxPython PUBLIC -DMASK_VERSION=0x57B0)
target_link_libraries(knxPython PUBLIC knx)
install(TARGETS knxPython LIBRARY DESTINATION .)