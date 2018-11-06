#include "button.h"
#include "state.h"
#include "knx_facade.h"

#ifdef USE_STATES
unsigned long buttonTimestamp = 0;

void buttonDown()
{
    buttonTimestamp = millis();
    attachInterrupt(knx.buttonPin(), buttonUp, RISING);
}
#endif

void buttonUp()
{
#ifdef USE_STATES
    if (millis() - buttonTimestamp > 1000)
    {
        Serial.println("long button press");
        currentState->longButtonPress();
    }
    else
    {
        Serial.println("short button press");
        currentState->shortButtonPress();
    }
    attachInterrupt(knx.buttonPin(), buttonDown, FALLING);
#else    if (knx.progMode())
    {
        digitalWrite(knx.ledPin(), LOW);
        knx.progMode(false);
    }
    else
    {
        digitalWrite(knx.ledPin(), HIGH);
        knx.progMode(true);
    }
#endif
}