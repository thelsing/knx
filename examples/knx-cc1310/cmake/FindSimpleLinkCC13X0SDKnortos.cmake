## Derived from this project: https://github.com/jobroe/cmake-arm-embedded
##
## MIT License
##
## Copyright (c) 2018 Johannes Bruder
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
## 
## The above copyright notice and this permission notice shall be included in all
## copies or substantial portions of the Software.
## 
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.

##
## Find TI's SimpleLink CC13X0 SDK
## 

include(FindPackageHandleStandardArgs)

find_path(SimpleLinkCC13X0SDK_DEVICES_DIR NAMES "DeviceFamily.h" PATH_SUFFIXES "source/ti/devices")

# Add suffix when looking for libraries
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES "am3g" "lib")

# Find TI's drivers lib
find_library(SimpleLinkCC13X0SDK_drivers_cc13x0_LIBRARY
    NAMES drivers_cc13x0.am3g
    PATH_SUFFIXES "source/ti/drivers/lib"
)

# Find cc13x0 radio single mode lib
find_library(SimpleLinkCC13X0SDK_rf_singleMode_cc13x0_LIBRARY
    NAMES rf_singleMode_cc13x0.am3g
    PATH_SUFFIXES "source/ti/drivers/rf/lib"
)

# Find cc13x0 radio multi mode lib
find_library(SimpleLinkCC13X0SDK_rf_multiMode_cc13x0_LIBRARY
    NAMES rf_multiMode_cc13x0.am3g
    PATH_SUFFIXES "source/ti/drivers/rf/lib"
)

# Find driver porting layer (NoRTOS) lib
find_library(SimpleLinkCC13X0SDK_dpl_cc13x0_LIBRARY
    NAMES nortos_cc13x0.am3g
    PATH_SUFFIXES "kernel/nortos/lib"
)

# Find Driverlib
find_library(SimpleLinkCC13X0SDK_driverlib_LIBRARY
    NAMES driverlib.lib
    PATH_SUFFIXES "source/ti/devices/cc13x0/driverlib/bin/gcc"
)

set(SimpleLinkCC13X0SDK_INCLUDE_DIRS
    "${SimpleLinkCC13X0SDK_DEVICES_DIR}/../.."
    "${SimpleLinkCC13X0SDK_DEVICES_DIR}/../../../kernel/nortos"
)

# Handle arguments and set SimpleLinkCC13X0SDK_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(SimpleLinkCC13X0SDK DEFAULT_MSG
    SimpleLinkCC13X0SDK_drivers_cc13x0_LIBRARY
    SimpleLinkCC13X0SDK_rf_singleMode_cc13x0_LIBRARY
    SimpleLinkCC13X0SDK_dpl_cc13x0_LIBRARY
    SimpleLinkCC13X0SDK_driverlib_LIBRARY
    SimpleLinkCC13X0SDK_INCLUDE_DIRS
)

set(SimpleLinkCC13X0SDK_LIBRARIES
    ${SimpleLinkCC13X0SDK_drivers_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_rf_singleMode_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_dpl_cc13x0_LIBRARY}
    ${SimpleLinkCC13X0SDK_driverlib_LIBRARY}
)
