#include <knx.h>
#include <knx/bau07B0.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <WiFiManager.h>
#endif

// create named references for easy access to group objects
#define goCurrent knx.getGroupObject(1)
#define goMax knx.getGroupObject(2)
#define goMin knx.getGroupObject(3)
#define goReset knx.getGroupObject(4)

// If you don't want a global knx object, for example because you want
// to more finely control it's construction, this is an example
// of how to do so. Define KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
// and then you can DIY a knx object as shown below. In this case we use 
// the ESP32's secondary UART and late-bind the ISR function in setup().
Esp32Platform knxPlatform(&Serial2);
Bau07B0 knxBau(knxPlatform);
KnxFacade<Esp32Platform, Bau07B0> knx(knxBau);

ICACHE_RAM_ATTR void myButtonPressed()
{
    // Debounce
    static uint32_t lastpressed=0;
    if (millis() - lastpressed > 200)
    {
        knx.toggleProgMode();
        lastpressed = millis();
    }
}

float currentValue = 0;
float maxValue = 0;
float minValue = RAND_MAX;
long lastsend = 0;

void measureTemp()
{
    long now = millis();
    if ((now - lastsend) < 2000)
        return;

    lastsend = now;
    int r = rand();
    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100 * 100;

    // write new value to groupobject
    goCurrent.value(currentValue);

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        goMax.value(maxValue);
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        goMin.value(minValue);
    }
}

// callback from reset-GO
void resetCallback(GroupObject& go)
{
    if (go.value())
    {
        maxValue = 0;
        minValue = 10000;
    }
}

void setup()
{
    knx.setButtonISRFunction(myButtonPressed);

    Serial.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial;

    Serial2.begin(19200);  // KNX, pin 16,17 on EPS32

    randomSeed(millis());

#ifdef ARDUINO_ARCH_ESP8266
    WiFiManager wifiManager;
    wifiManager.autoConnect("knx-demo");
#endif

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // print values of parameters if device is already configured
    if (knx.configured())
    {
        // register callback for reset GO
        goReset.callback(resetCallback);
        goReset.dataPointType(DPT_Trigger);
        goCurrent.dataPointType(DPT_Value_Temp);
        goMin.dataPointType(DPT_Value_Temp);
        goMax.dataPointType(DPT_Value_Temp);

        Serial.print("Timeout: ");
        Serial.println(knx.paramByte(0));
        Serial.print("Zykl. senden: ");
        Serial.println(knx.paramByte(1));
        Serial.print("Min/Max senden: ");
        Serial.println(knx.paramByte(2));
        Serial.print("Aenderung senden: ");
        Serial.println(knx.paramByte(3));
        Serial.print("Abgleich: ");
        Serial.println(knx.paramByte(4));
    }

    // pin or GPIO the programming led is connected to. Default is LED_BUILTIN
    // knx.ledPin(LED_BUILTIN);
    // is the led active on HIGH or low? Default is LOW
    // knx.ledPinActiveOn(HIGH);
    // pin or GPIO programming button is connected to. Default is 0
    // knx.buttonPin(0);

    // start the framework.
    knx.start();
}

void loop()
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (!knx.configured())
        return;

    measureTemp();
}
