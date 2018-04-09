#include "led.h"
#include "knx_facade.h"
#include "state.h"

void doLed()
{
    if (!currentState)
        return;

    if (!currentState->ledOn())
    {
        digitalWrite(knx.ledPin(), HIGH);
        return;
    }

    unsigned int period = currentState->blinkPeriod();

    if (!currentState->ledBlink() || period == 0)
    {
        digitalWrite(knx.ledPin(), LOW);
        return;
    }

    if ((millis() % period) > (period / 2))
        digitalWrite(knx.ledPin(), HIGH);
    else
        digitalWrite(knx.ledPin(), LOW);
}