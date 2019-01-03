#include "runningstate.h"
#include "programmingmodestate.h"
#include "wpsstate.h"
#include "knx_facade.h"

#ifdef USE_STATES

RunningState runningState = RunningState();

void RunningState::shortButtonPress()
{
    switchToSate(programModeState);
}

void RunningState::longButtonPress()
{
    switchToSate(wpsState);
}

void RunningState::enterState()
{
    if (_initialized)
        return;

    knx.enabled(true);
    _initialized = true;
}

void RunningState::leaveState()
{
    if (nextState != &programModeState)
    {
        _initialized = false;
        knx.enabled(false);
    }
}

void RunningState::loop()
{
    State::loop();
    knx.knxLoop();
}
#endif