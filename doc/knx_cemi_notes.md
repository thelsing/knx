KNX cEMI Server 
===============

Implementation Notes
--------------------
* currently only implemented for KNX USB Data Interface (Tunnel) and not KNXnet/IP for now

* basically provides a complete KNX device (TP or RF) that also has a USB interface built-in which can be used to program either the local device stack or 
remote devices via the attached medium (TP or RF).
  * tested with ETS 5.7.x
  * tested with Net'N'Node
  * **Be careful when using this with TP as your HW setup might not have a galvanic isolation between the KNX bus and USB!**
    * An [USB isolator](https://www.olimex.com/Products/USB-Modules/USB-ISO/) might be the easiest option. Not tested!
    * ToDo: How is this realized in commercial KNX USB Data Interfaces?

* cEMI client address (e.g. ETS) is based on KNX physical address of the local device stack +1
  * Example: local device: 1.2.10 -> cEMI client address: 1.2.11
  * PropertyWrite commands to change the cEMI client address are only temporary until next device restart

* requires a USB stack which properly work with USB HID and provides interrupt IN/OUT endpoints for KNX HID reports.
  * normal Arduino USB device stack does NOT work as it is missing required functionality
  * TinyUSB stack is used instead, therefore the Adafruit SAMD core with TinyUSB enabled is used as the Arduino core

Development environment
-----------------------
* PlatformIO
* Segger J-Link EDU
* [GY-SAMD21 Board](https://eckstein-shop.de/GY-SAMD21-Mini-Breakout-fuer-Arduino), compatible to [Adafruit Feather M0](https://www.adafruit.com/product/2772)

