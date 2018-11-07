#include "programmingmodestate.h"
#include "runningstate.h"
#include "knx_facade.h"

#ifdef USE_STATES
ProgramModeState programModeState = ProgramModeState();

void ProgramModeState::enterState()
{
    knx.progMode(true);
}

void ProgramModeState::leaveState()
{
    knx.progMode(false);
}

void ProgramModeState::shortButtonPress()
{
    switchToSate(runningState);
}

void ProgramModeState::loop()
{
    State::loop();
    knx.knxLoop();
}
#endif