#pragma once

#include "state.h"

#ifdef USE_STATES

class ProgramModeState : public State
{
public:
    ProgramModeState() : State(true, true, 200)
    {}
    virtual void enterState();
    virtual void leaveState();
    virtual void shortButtonPress();
    virtual void loop();
    virtual const char* name() { return "ProgramMode"; }
};

extern ProgramModeState programModeState;

#endif