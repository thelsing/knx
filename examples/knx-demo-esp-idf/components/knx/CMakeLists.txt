# Define the directory containing your source files
set(SOURCE_DIR "../../../../src")
set(SOURCE_DIR_1 "../../../../src/knx")

# Use file(GLOB) to find all .cpp files in the 'src' directory
file(GLOB SOURCE_FILES "${SOURCE_DIR}/*.cpp")
file(GLOB SOURCE_FILES_1 "${SOURCE_DIR_1}/*.cpp")

idf_component_register(SRCS ${SOURCE_FILES} ${SOURCE_FILES_1}
                    INCLUDE_DIRS "../../../../src" "../../../../src/knx"
                    REQUIRES esp_netif driver esp_timer esp_wifi freertos nvs_flash esp_system) 