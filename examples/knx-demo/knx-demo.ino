#include <knx.h>


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
    goCurrent.objectWriteFloatDpt9(currentValue);

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        goMax.objectWriteFloatDpt9(maxValue);
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        goMin.objectWriteFloatDpt9(minValue);
    }
}

// callback from reset-GO
void resetCallback(GroupObject& go)
{
    if (go.objectReadBool())
    {
        maxValue = 0;
        minValue = 10000;
    }
}

void setup()
{
    SerialDBG.begin(115200);

    randomSeed(millis());

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // print values of parameters if device is already configured
    if (knx.configured())
    {
        // register callback for reset GO
        goReset.callback(resetCallback);
        
        SerialDBG.print("Timeout: "); SerialDBG.println(knx.paramByte(0));
        SerialDBG.print("Zykl. senden: "); SerialDBG.println(knx.paramByte(1));
        SerialDBG.print("Min/Max senden: "); SerialDBG.println(knx.paramByte(2));
        SerialDBG.print("Aenderung senden: "); SerialDBG.println(knx.paramByte(3));
        SerialDBG.print("Abgleich: "); SerialDBG.println(knx.paramByte(4));
    }

    // start the framework. Will get wifi first.
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