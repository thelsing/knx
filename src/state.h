#pragma once
#include "arch_config.h"

#ifdef USE_STATES

class State
{
public:
    State(bool led, bool blink, int period) :
        _ledOn(led), _ledBlink(blink), _blinkPeriod(period)
    {}
    virtual ~State() {}
    bool ledOn();
    bool ledBlink();
    unsigned int blinkPeriod();
    virtual void shortButtonPress() {}
    virtual void longButtonPress() {}
    virtual void enterState() {}
    virtual void leaveState() {}
    virtual void loop();
    virtual const char* name() = 0;
private:
    bool _ledOn;
    bool _ledBlink;
    int _blinkPeriod;
};

void switchToSate(State& state);
void checkStates();

extern State* volatile currentState;
extern State* volatile nextState;

#endif