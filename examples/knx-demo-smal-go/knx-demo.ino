#include <knx.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <WiFiManager.h>
#endif

/*****************************************
 * changes necessary for SMALL_GROUPOBJECT
 * are commented with //** 
 * This project can be used with any
 * of the knxprod files of the original
 * knx-demo project.
 *****************************************/

// create named references for easy access to group objects
#define goCurrent knx.getGroupObject(1)
#define goMax knx.getGroupObject(2)
#define goMin knx.getGroupObject(3)
#define goReset knx.getGroupObject(4)

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
    goCurrent.value(currentValue, DPT_Value_Temp); //** each value access needs to done with according DPT parameter

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        goMax.value(maxValue, DPT_Value_Temp); //** each value access needs to done with according DPT parameter
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        goMin.value(minValue, DPT_Value_Temp); //** each value access needs to done with according DPT parameter
    }
}

// callback from reset-GO
void resetCallback(GroupObject& go)
{
    //** callbacks are now handled in the class, not per instance,
    //** this means, we have to check, which GroupObject is calling back
    if (go.asap() == goReset.asap())
    {
        if (go.value(DPT_Trigger)) //** each value access needs to done with according DPT parameter
        {
            maxValue = 0;
            minValue = 10000;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial;

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
        GroupObject::classCallback(resetCallback); //** callbacks are now handled per class, not per instance
        //** there is no global assignment of DPT for GroupObjects
        // goReset.dataPointType(DPT_Trigger);
        // goCurrent.dataPointType(DPT_Value_Temp);
        // goMin.dataPointType(DPT_Value_Temp);
        // goMax.dataPointType(DPT_Value_Temp);

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
    // Is the interrup created in RISING or FALLING signal? Default is RISING
    // knx.buttonPinInterruptOn(FALLING);

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