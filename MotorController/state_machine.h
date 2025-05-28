#ifndef __MC_STATE_MACHINE_H__
#define __MC_STATE_MACHINE_H__
#include "stdbool.h"
#include "foc_type.h"

void sm_init(statemachine_t *stateMachine);
bool sm_nextState(statemachine_t *stateMachine, state_t state);
state_t sm_getState(statemachine_t *stateMachine);

#endif