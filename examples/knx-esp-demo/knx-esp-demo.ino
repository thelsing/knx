#include <EEPROM.h>
#include <knx_esp.h>

// declare array of all groupobjects with their sizes in byte
GroupObject groupObjects[]
{
    GroupObject(2),
    GroupObject(2),
    GroupObject(2),
    GroupObject(1)
};

// create named references for easy access to group objects
GroupObject& goCurrent = groupObjects[0];
GroupObject& goMax = groupObjects[1];
GroupObject& goMin = groupObjects[2];
GroupObject& goReset = groupObjects[3];

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
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    randomSeed(millis());

    // register group objects
    knx.registerGroupObjects(groupObjects, 4);
    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // register callback for reset GO
    goReset.updateHandler = resetCallback;

    // print values of parameters if device is already configured
    if (knx.configured())
    {
        Serial.printf("Timeout: %d\n", knx.paramByte(0));
        Serial.printf("Zykl. senden: %d\n", knx.paramByte(1));
        Serial.printf("Min/Max senden: %d\n", knx.paramByte(2));
        Serial.printf("Aenderung senden: %d\n", knx.paramByte(3));
        Serial.printf("Abgleich %d\n", knx.paramByte(4));
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
