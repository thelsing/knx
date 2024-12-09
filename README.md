# knx

This is a fork of the thelsing/knx stack from Thomas Kunze for and by the OpenKNX Team.

While we did not remove support for any plattform, the testing focus is on RP2040 (main), ESP32 (experimental) and SAMD21(deprecated).

This projects provides a knx-device stack for arduino (ESP8266, ESP32, SAMD21, RP2040, STM32), CC1310 and linux. (more are quite easy to add)
It implements most of System-B specification and can be configured with ETS.
The necessary knxprod-files can be generated with the [Kaenx-Creator](https://github.com/OpenKNX/Kaenx-Creator) tool.


## Usage
See the examples for basic usage options


## Changelog

### v1dev (replace this with version and date when releasing to v1)
- Fix [#30](https://github.com/OpenKNX/knx/pull/30): Unexpected behaviour of `GroupObject` on failed conversion to DPT
  - `GroupObject::value[No]SendCompare(..)` resulted in value 0 (and returned change based on this value)
  - `GroupObject::valueNoSend(..)` updated state from unitialized to OK, without updating the value
  - `GroupObject::value(..)` wrote to GA without setting the KO value
- Extension [#30](https://github.com/OpenKNX/knx/pull/30): Return successful conversion to DPT on values update operations in `GroupObject` (changed result-type of some methods from `void` to `bool`) 
- only set pinMode of Prog button pin if valid (PROG_BUTTON_PIN >= 0)
- Strings are now \0 terminated in group objects (#25)
- change defines in the rp2040 plattform for LAN / WLAN usage to KNX_IP_LAN or KNX_IP_WIFI, remove KNX_IP_GENERIC
- update examples and ci to current arduino-pico core version
- better Routing and Tunneling support
- add DPT 27.001
- increase device object api version to 2 (invalidation of knx flash data stored by older versions)
- add #pragma once to Arduino plattform to allow derived plattforms
- change esp32 plattform to use KNX_NETIF
- fix out-of-boundary write and dereferenced nullpointer access in tunneling support

### V2.1.2 - 2024-12-09
- adds unicast auto ack

### V2.1.1 - 2024-09-16
- fix minor bug in TP-Uart Driver (RX queue out of boundary)

### V2.1.0 - 2024-07-03
- complete rework of the TPUart DataLinkLayer with support interrupt-based handling and optimized queue handling
- added DMA support for RP2040 platform
- fix some issues with continous integration causing github actions to fail
- added rp2040 plattform to knx-demo example
- added bool GroupObject::valueCompare method for only sending the value when it has changed 

### V2.0.0 - 2024-02-13
- first OpenKNX version