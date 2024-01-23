# knx

This projects provides a knx-device stack for arduino (ESP8266, ESP32, SAMD21, RP2040, STM32), CC1310 and linux. (more are quite easy to add)
It implements most of System-B specification and can be configured with ETS.
The necessary knxprod-files can be generated with the [Kaenx-Creator](https://github.com/OpenKNX/Kaenx-Creator) tool.

For ESP8266 and ESP32 [WifiManager](https://github.com/tzapu/WiFiManager) is used to configure wifi.

Don't forget to reset ESP8266 manually (disconnect power) after flashing. The reboot doen't work during configuration with ETS otherwise.

Generated documentation can be found [here](https://knx.readthedocs.io/en/latest/).

## Stack configuration possibilities

Specify prog button GPIO other then `GPIO0`:
```C++
knx.buttonPin(3); // Use GPIO3 Pin
```

Specify a LED GPIO for programming mode other then the `LED_BUILTIN`:
```C++
knx.ledPin(5);
```

Use a custom function instead of a LED connected to GPIO to indicate the programming mode:
```C++
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <knx.h>
// create a pixel strand with 1 pixel on PIN_NEOPIXEL
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);

void progLedOff()
{
  pixels.clear();
  pixels.show();
}

void progLedOn()
{
  pixels.setPixelColor(0, pixels.Color(20, 0, 0));
  pixels.show();
}

void main ()
{
 knx.setProgLedOffCallback(progLedOff);
 knx.setProgLedOnCallback(progLedOn);
 [...]
}
```

More configuration options can be found in the examples. 
