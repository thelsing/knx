#pragma once

#include "state.h"

#ifdef USE_STATES
class WpsState : public State
{
public:
    WpsState() : State(true, true, 400)
    {}
    virtual void enterState();
    virtual const char* name() { return "Wps"; }
};

extern WpsState wpsState;

#endif
