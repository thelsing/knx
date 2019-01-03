#ifdef __linux__

#include "linux_platform.h"
#include "knx/bau57B0.h"
#include "knx/group_object_table_object.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

LinuxPlatform platfrom;
Bau57B0 bau(platfrom);

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
#define CURR groupObjects[0]
#define MAX groupObjects[1]
#define MIN groupObjects[2]
#define RESET groupObjects[3]

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
    bau.readMemory();
    
    GroupObjectTableObject& got(bau.groupObjectTable());
    got.groupObjects(groupObjects, 4);
    
    RESET.callback(resetCallback);

    if (bau.deviceObject().induvidualAddress() == 0)
        bau.deviceObject().progMode(true);

    if (bau.parameters().loadState() == LS_LOADED)
    {
        printf("Timeout: %d\n", bau.parameters().getWord(0));
        printf("Zykl. senden: %d\n", bau.parameters().getByte(2));
        printf("Min/Max senden: %d\n", bau.parameters().getByte(3));
        printf("Aenderung senden: %d\n", bau.parameters().getByte(4));
        printf("Abgleich %d\n", bau.parameters().getByte(5));
    }
    bau.enabled(true);
}

int main(int argc, char **argv)
{
    setup();
    
    while (1)
    {
        bau.loop();
        appLoop();
        platfrom.mdelay(100);
    }
}
#endif