#ifndef __FOC_APP_H
#define __FOC_APP_H
#include <stdint.h>
#include "foc_type.h"

void sincos_test(void);
void openloop_mode(void);
void set_duty(math_3f_t *vswitch);

void stateMachine_task(axit_t *axit);
void foc_app_init(void);
void mc_start(axit_t *axit);
void mc_stop(axit_t *axit);
void pwm_OutputDisable(void);
void pwm_OutputEnable(void);
void pwm_OutputLowSide(void);
void foc_math_test(void);
void foc_math_test_idq_const(void);



extern axit_t g_axit;

#endif