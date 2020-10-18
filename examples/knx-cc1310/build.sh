#cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH="~/ti/simplelink_cc13x0_sdk_4_10_02_04" -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-none-eabi.cmake ..
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-none-eabi.cmake ..
