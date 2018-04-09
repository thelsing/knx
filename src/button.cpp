#include "button.h"
#include "state.h"
#include "knx_facade.h"

unsigned long buttonTimestamp = 0;
void buttonUp()
{
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
}

void buttonDown()
{
    buttonTimestamp = millis();
    attachInterrupt(knx.buttonPin(), buttonUp, RISING);
}