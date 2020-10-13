#include "knx.h"
#include <cstdio>

#include "knx_wrapper.h"

KnxFacade<CC1310Platform, Bau07B0> *pKnx = nullptr;

void setup()
{
    pKnx = new KnxFacade<CC1310Platform, Bau07B0>;
    KnxFacade<CC1310Platform, Bau07B0> &knx = *pKnx;

    knx.platform().earlyInit();

    knx.readMemory();

    if (knx.induvidualAddress() == 0)
        knx.progMode(true);

    if (knx.configured())
    {
        printf("configured %d\n", knx.paramByte(5));
    }
    else
        println("not configured");
    knx.start();
}

void loop()
{
    KnxFacade<CC1310Platform, Bau07B0> &knx = *pKnx;

    knx.loop();
}
