#include "knx_facade.h"
#include "knx/bau57B0.h"
#include "knx/group_object_table_object.h"
#include "knx/bits.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

KnxFacade* knx = 0;
Platform* platform = 0;

long lastsend = 0;

#define CURR knx->getGroupObject(1)
#define MAX knx->getGroupObject(2)
#define MIN knx->getGroupObject(3)
#define RESET knx->getGroupObject(4)

void measureTemp()
{
    long now = platform->millis();
    if ((now - lastsend) < 10000)
        return;

    lastsend = now;
    int r = rand();
    float currentValue = (r * 1.0) / (RAND_MAX * 1.0);
    currentValue *= (670433.28 + 273);
    currentValue -= 273;
    println(currentValue);
    CURR.value(currentValue);

    double max = MAX.value();
    if (currentValue > max)
        MAX.value(currentValue);

    double min = MIN.value();
    if (currentValue < MIN.value().doubleValue())
        MIN.value(currentValue);
}

void resetCallback(GroupObject& go)
{
    if (go.value().boolValue())
    {
        MAX.valueNoSend(-273.0);
        MIN.valueNoSend(670433.28);
    }
}

void appLoop()
{
    if (!knx->configured())
        return;
    
    measureTemp();
}

void setup()
{
    srand((unsigned int)time(NULL));
    knx->readMemory();

    if (knx->induvidualAddress() == 0)
        knx->progMode(true);

    if (knx->configured())
    {
        CURR.dataPointType(Dpt(9, 1));
        MIN.dataPointType(Dpt(9, 1));
        MIN.value(670433.28);
        MAX.dataPointType(Dpt(9, 1));
        MAX.valueNoSend(-273.0);
        RESET.dataPointType(Dpt(1, 15));
        RESET.callback(resetCallback);
        printf("Timeout: %d\n", knx->paramWord(0));
        printf("Zykl. senden: %d\n", knx->paramByte(2));
        printf("Min/Max senden: %d\n", knx->paramByte(3));
        printf("Aenderung senden: %d\n", knx->paramByte(4));
        printf("Abgleich %d\n", knx->paramByte(5));
    }
    knx->start();
}

int main(int argc, char **argv)
{
    platform = new LinuxPlatform(argc, argv);
    Bau57B0 bau(*platform);
    knx = new KnxFacade(bau);
    
    setup();
    
    while (1)
    {
        knx->loop();
        if(knx->configured())
            appLoop();
        platform->mdelay(100);
    }
}