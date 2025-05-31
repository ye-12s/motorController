#include "foc_app.h"
#include "foc_math.h"
#include "foc_param.h"
#include "foc_type.h"
#include "mt6701.h"
#include "state_machine.h"
#include "stm32g4xx_hal.h"
#include "vofa.h"
#include <string.h>

char upper_cmd[128];

void mc_start(axit_t *axit)
{
    statemachine_nextState(&axit->stateMachine, S_IDLE_DONE);
}

void mc_stop(axit_t *axit)
{
    statemachine_nextState(&axit->stateMachine, S_ANY_STOP);
}

int foc_cmd_resolver(const char *cmd, uint32_t len)
{
    // printk("foc_cmd_resolver: %.*s", len, cmd);
    if (len >= sizeof(upper_cmd)) {
        return -1; // Command too long
    }

    if (memcmp(cmd, "start", 5) == 0) {
        mc_start(&g_axit);
    } else if (memcmp(cmd, "stop", 4) == 0) {
        mc_stop(&g_axit);
    }

    return 0;
}
