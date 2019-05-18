#include "knx_facade.h"
#include "knx/bau57B0.h"
#include "knx/group_object_table_object.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

LinuxPlatform platfrom;
Bau57B0 bau(platfrom);
KnxFacade knx(bau);

float currentValue = 0;
float maxValue = 0;
float minValue = RAND_MAX;
long lastsend = 0;

#define CURR knx.getGroupObject(1)
#define MAX knx.getGroupObject(2)
#define MIN knx.getGroupObject(3)
#define RESET knx.getGroupObject(4)

void measureTemp()
{
    long now = platfrom.millis();
    if ((now - lastsend) < 2000)
        return;

    lastsend = now;
    int r = rand();
    currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= 100 * 100;
    

    CURR.objectWrite(currentValue);

    if (currentValue > maxValue)
    {
        maxValue = currentValue;
        MAX.objectWrite(maxValue);
    }

    if (currentValue < minValue)
    {
        minValue = currentValue;
        MIN.objectWrite(minValue);
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

void appLoop()
{
    if (!bau.configured())
        return;
    
    measureTemp();
}

void setup()
{
    srand((unsigned int)time(NULL));
    knx.readMemory();

    if (knx.induvidualAddress() == 0)
        knx.progMode(true);

    if (knx.configured())
    {
        RESET.callback(resetCallback);
        printf("Timeout: %d\n", bau.parameters().getWord(0));
        printf("Zykl. senden: %d\n", bau.parameters().getByte(2));
        printf("Min/Max senden: %d\n", bau.parameters().getByte(3));
        printf("Aenderung senden: %d\n", bau.parameters().getByte(4));
        printf("Abgleich %d\n", bau.parameters().getByte(5));
    }
    knx.start();
}

int main(int argc, char **argv)
{
    setup();
    
    while (1)
    {
        knx.loop();
        if(knx.configured())
            appLoop();
        platfrom.mdelay(100);
    }
}