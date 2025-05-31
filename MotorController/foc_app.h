#ifndef __FOC_APP_H
#define __FOC_APP_H
#include <stdint.h>
#include "foc_type.h"

void openloop_mode(void);
void set_duty(math_3f_t *vswitch);

void stateMachine_task(axit_t *axit);
void foc_app_init(void);
void mc_start(axit_t *axit);
void mc_stop(axit_t *axit);
void pwm_OutputDisable(void);
void pwm_OutputEnable(void);
void pwm_OutputLowSide(void);


int foc_cmd_resolver(const char *cmd, uint32_t len);



extern axit_t g_axit;

#endif