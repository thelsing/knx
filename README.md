knx  [![Build Status](https://travis-ci.org/thelsing/knx.svg?branch=master)](https://travis-ci.org/thelsing/knx)
===



This projects provides a knx-device stack for arduino (ESP8266, SAMD21) and linux. (more are quite easy to add)
It implements most of System-B specification and can be configured with ETS.
The necessary knxprod-files can be generated with my [CreateKnxProd](https://github.com/thelsing/CreateKnxProd) tool.

For esp8266 [WifiManager](https://github.com/tzapu/WiFiManager) is used to configure wifi.

Don't forget to reset ESP8266 manually (disconnect power) after flashing. The reboot doen't work during configuration with ETS otherwise.

The SAMD21 version uses my version of the [FlashStorage](https://github.com/thelsing/FlashStorage) lib (Pull request pending).

Generated documentation can be found [here](https://knx.readthedocs.io/en/latest/).
