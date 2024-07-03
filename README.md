# knx

This is a fork of the thelsing/knx stack from Thomas Kunze for and by the OpenKNX Team.

While we did not remove support for any plattform, the testing focus is on RP2040 (main), ESP32 (experimental) and SAMD21(deprecated).

This projects provides a knx-device stack for arduino (ESP8266, ESP32, SAMD21, RP2040, STM32), CC1310 and linux. (more are quite easy to add)
It implements most of System-B specification and can be configured with ETS.
The necessary knxprod-files can be generated with the [Kaenx-Creator](https://github.com/OpenKNX/Kaenx-Creator) tool.


## Usage
See the examples for basic usage options


## Changelog

### V2.1.0 - 2024-07-03
- complete rework of the TPUart DataLinkLayer with support interrupt-based handling and optimized queue handling
- added DMA support for RP2040 platform
- fix some issues with continous integration causing github actions to fail
- added rp2040 plattform to knx-demo example
- added bool GroupObject::valueCompare method for only sending the value when it has changed 

### V2.0.0 - 2024-02-13
- first OpenKNX version