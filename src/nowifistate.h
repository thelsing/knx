#pragma once

#include "state.h"

class NoWifiState : public State
{
public:
    NoWifiState() : State(true, false, 0)
    {}
    virtual void shortButtonPress();
    virtual void longButtonPress();
    virtual void enterState();
    virtual const char* name() { return "NoWifi"; }
};

extern NoWifiState noWifiState;