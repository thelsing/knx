set(COMPONENT_SRCDIRS
"src"
"src/knx"
)
 
set(COMPONENT_ADD_INCLUDEDIRS
"src"
"src/knx"
)
 
set(COMPONENT_REQUIRES
"arduino-esp32"
"libsodium"
)
 
register_component()
 
target_compile_definitions(${COMPONENT_TARGET} PUBLIC -DESP32)
target_compile_options(${COMPONENT_TARGET} PRIVATE -fno-rtti)  