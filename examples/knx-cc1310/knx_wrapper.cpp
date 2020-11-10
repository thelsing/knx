#include "knx.h"
#include <cstdio>

#include "knx_wrapper.h"

KnxFacade<CC1310Platform, Bau27B0> *pKnx = nullptr;

void buttonUp()
{
    static uint32_t lastpressed=0;
    if (millis() - lastpressed > 200)
    {
        KnxFacade<CC1310Platform, Bau27B0> &knx = *pKnx;
        knx._toogleProgMode = true;
        lastpressed = millis();
    }
}

void setup()
{
    pKnx = new KnxFacade<CC1310Platform, Bau27B0>;
    KnxFacade<CC1310Platform, Bau27B0> &knx = *pKnx;

    // see GPIO_PinConfig gpioPinConfigs[]
    knx.buttonPin(0);
    knx.ledPinActiveOn(HIGH);

    knx.platform().init();

    knx.readMemory();

    if (knx.individualAddress() == 0)
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
    KnxFacade<CC1310Platform, Bau27B0> &knx = *pKnx;

    knx.loop();
}
