#!/bin/sh

~/.platformio/penv/bin/platformio ci --lib="." --project-conf=examples/knx-usb/platformio-ci.ini examples/knx-usb/src/main.cpp && ~/.platformio/penv/bin/platformio ci --lib="." --project-conf=examples/knx-demo/platformio-ci.ini examples/knx-demo/knx-demo.ino && ~/.platformio/penv/bin/platformio ci --lib="." --project-conf=examples/knx-demo-coupler/platformio-ci.ini examples/knx-demo-coupler/knx-demo-coupler.ino
