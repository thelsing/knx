#include <EEPROM.h>
#include "knx_esp.h"

float currentValue = 0;
float maxValue = 0;
float minValue = RAND_MAX;
long lastsend = 0;

GroupObject groupObjects[]
{
    GroupObject(2),
    GroupObject(2),
    GroupObject(2),
    GroupObject(1)
};

GroupObject& goCurrent = groupObjects[0];
GroupObject& goMax = groupObjects[1];
GroupObject& goMin = groupObjects[2];
GroupObject& goReset = groupObjects[3];

void measureTemp()
{
    long now = millis();
    if ((now - lastsend) < 2000)
        return;

    lastsend = now;
    int r = rand();
    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100 * 100;

    goCurrent.objectWriteFloat(currentValue);

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        goMax.objectWriteFloat(maxValue);
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        goMin.objectWriteFloat(minValue);
    }
}

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

    knx.registerGroupObjects(groupObjects, 4);
    knx.readMemory();

    goReset.updateHandler = resetCallback;

    if (knx.configured())
    {
        Serial.printf("Timeout: %d\n", knx.paramByte(0));
        Serial.printf("Zykl. senden: %d\n", knx.paramByte(1));
        Serial.printf("Min/Max senden: %d\n", knx.paramByte(2));
        Serial.printf("Aenderung senden: %d\n", knx.paramByte(3));
        Serial.printf("Abgleich %d\n", knx.paramByte(4));
    }
    
    knx.start();
}

void loop() 
{
    knx.loop();
    if (!knx.configured())
        return;

    measureTemp();
}
