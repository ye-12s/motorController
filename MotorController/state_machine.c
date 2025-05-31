#include "state_machine.h"
#include "stdbool.h"

void statemachine_init(statemachine_t *stateMachine)
{
    stateMachine->state = S_IDLE;
}

bool statemachine_nextState(statemachine_t *stateMachine, state_t state)
{
    bool chargState   = false;
    state_t currState = stateMachine->state;
    state_t nextState = currState;

    switch (currState) {
        case S_IDLE:
            if (state == S_IDLE_DONE) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_IDLE_DONE:
            if (state == S_ANY_STOP || state == S_CHARG_BOOT_CAP || state == S_START || state == S_OFFICE_CALIB) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_CHARG_BOOT_CAP:
            if (state == S_OFFICE_CALIB || state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_OFFICE_CALIB:
            if (state == S_ALIGN || state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_ALIGN:
            if (state == S_CLEAR || state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_CLEAR:
            if (state == S_START || state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_START:
            if (state == S_START_DONE || state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_START_DONE:
            if (state == S_RUN || state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_RUN:
            if (state == S_ANY_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_ANY_STOP:
            if (state == S_STOP) {
                chargState = true;
                nextState  = state;
            }
            break;
        case S_STOP:
            if (state == S_IDLE) {
                chargState = true;
                nextState  = state;
            }
            break;
        default:
            break;
    }

    if (chargState) {
        stateMachine->state = nextState;
    }
    return chargState;
}

state_t statemachine_getState(statemachine_t *stateMachine)
{
    return stateMachine->state;
}
