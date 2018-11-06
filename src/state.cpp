#include "state.h"
#include "Arduino.h"

#ifdef USE_STATES

State* volatile currentState = 0;
State* volatile nextState = 0;

void switchToSate(State& state)
{
    nextState = &state;
}

void checkStates()
{
    if (!nextState)
        return;

    if (nextState == currentState)
        return;

    if (currentState)
    {
        printf("Leave %s\n", currentState->name());
        currentState->leaveState();
    }

    currentState = nextState;

    if (currentState)
    {
        printf("Enter %s\n", currentState->name());
        currentState->enterState();
    }
}

bool State::ledOn()
{
    return _ledOn;
}

bool State::ledBlink()
{
    return _ledBlink;
}

unsigned int State::blinkPeriod()
{
    return _blinkPeriod;
}

void State::loop()
{
    checkStates();
}

#endif