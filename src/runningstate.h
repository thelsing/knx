#pragma once

#include "state.h"

class RunningState : public State
{
public:
    RunningState() : State(false, false, 0)
    {}
    virtual void shortButtonPress();
    virtual void longButtonPress();
    virtual void enterState();
    virtual void leaveState();
    virtual void loop();
    virtual const char* name() { return "Running"; }
private:
    bool _initialized = false;
};

extern RunningState runningState;