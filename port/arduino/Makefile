#
# Makefile to setup KNX stack library in Arduino folder (OS X/Linux only) and create .zip archive
#

DIR=.
KNXSTACK_ROOT=${DIR}/../..
DUMMY=$(shell )
#VERSION=`sed -n -e 's/^.*BTSTACK_VERSION \"\(.*\)\"/\1/p' ${KNXSTACK_ROOT}/platform/daemon/src/btstack_version.h`
VERSION=1.0.0
KNXSTACK_PACKAGE=/tmp/knxstack
ARCHIVE=knx-arduino-${VERSION}.zip

SRC_FILES  = address_table_object.cpp  application_program_object.cpp  bau57B0.cpp      bits.cpp             datapoint_types.cpp  group_object_table_object.cpp  ip_parameter_object.cpp  npdu.cpp          tpdu.cpp
SRC_FILES += apdu.cpp                  association_table_object.cpp    bau.cpp          cemi_frame.cpp       device_object.cpp    interface_object.cpp           memory.cpp               platform.cpp      tpuart_data_link_layer.cpp
SRC_FILES += application_layer.cpp     bau07B0.cpp                     bau_systemB.cpp  data_link_layer.cpp  group_object.cpp     ip_data_link_layer.cpp         network_layer.cpp        table_object.cpp  transport_layer.cpp
PORT_FILES =
PLATFORM_FILES = button.cpp esp_platform.cpp knx_facade.cpp led.cpp nowifistate.cpp programmingmodestate.cpp runningstate.cpp samd_platform.cpp state.cpp wpsstate.cpp

PATHS  = $(addprefix ${KNXSTACK_ROOT}/src/knx/, ${SRC_FILES})
PATHS += $(filter-out ${KNXSTACK_ROOT}/src/knx/knxstack.h, $(wildcard ${KNXSTACK_ROOT}/src/knx/*.h))
PATHS += $(addprefix ${KNXSTACK_ROOT}/platform/arduino/, ${PLATFORM_FILES})
PATHS += $(wildcard  ${KNXSTACK_ROOT}/platform/arduino/*.h)
PATHS += ${KNXSTACK_ROOT}/port/arduino/examples
PATHS += $(addprefix ${DIR}/, ${PORT_FILES})
PATHS += ${KNXSTACK_ROOT}/library.properties

ARDUINO_LIBS=~/Documents/arduino/libraries/knx

all: release

clean:
	rm -rf ${KNXSTACK_PACKAGE}

update_version:
#	${KNXSTACK_ROOT}/tool/get_version.sh 

install: update_version
	rm -rf ${ARDUINO_LIBS}
	mkdir ${ARDUINO_LIBS}
	cp -r ${PATHS} ${ARDUINO_LIBS}

release: update_version
	rm -rf ${KNXSTACK_PACKAGE}
	mkdir ${KNXSTACK_PACKAGE}
	cp -r ${PATHS} ${KNXSTACK_PACKAGE}
	rm -f ${ARCHIVE}
	zip -r ${ARCHIVE} ${KNXSTACK_PACKAGE}
	cp ${ARCHIVE} knx-arduino-latest.zip
