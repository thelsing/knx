This projects provides a knx-device stack for arduino (ESP8266, SAMD21) and linux. (more are quite easy to add)
It implements most of System-B specification and can be configured with ETS.
The necessary knxprod-files can be generated with my [CreateKnxProd](https://github.com/thelsing/CreateKnxProd) tool.

The visual studio files use the [VisualGdb](https://visualgdb.com/). But you can use the lib in Arduino as usual. 
The linux Version can be build with cmake.

For esp8266 WPS is used. Current esp8266 arduino core disables WPS. I forgot how to reenable it. But google should be able to help. ;)
The stack does the following on esp8266:
1. Boot. If Wifi is configured goto 3. Led is on. On short button press goto 2
2. Try to connecto to wifi with WPS. If successful goto 3 else goto 1 (Led blinks)
3. App running. (Led off) On short button press goto 4. On long button press goto 2
4. Programming mod. (Led blinks fast). On short button press goto 3.
This behavior can be disabled in arch_config.h (remove the #define USE_STATES)

Don't forget to reset ESP8266 manually (disconnect power) after flashing. The reboot doen't work during configuration with ETS otherwise.

The SAMD21 version uses my version of the [FlashStorage](https://github.com/thelsing/FlashStorage) lib (Pull request pending).
