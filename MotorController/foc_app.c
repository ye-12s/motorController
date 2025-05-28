#include "foc_app.h"
#include "foc_math.h"
#include "foc_type.h"
#include "mt6701.h"
#include "stm32g4xx_hal.h"
#include "vofa.h"
#include "arm_math.h" // Add this include for arm_sin_f32 and arm_cos_f32

float vangle = 0;
float vamp   = 0.1;

void vf_mode(void)
{
    math_2f_t valbe   = {0};
    math_3f_t vabc    = {0};
    math_3f_t vswTime = {0};
    vangle += 0.1f;
    if (vangle > 2 * PI) {
        vangle -= 2 * PI;
    }

    decompAlBe(vamp, vangle, &valbe);
    svpwm(&valbe, &vswTime, &vabc);
    set_duty(&vswTime);

    vofa_set_channel(0, vangle);
    vofa_set_channel(3, valbe.arg1);
    vofa_set_channel(4, valbe.arg2);
    vofa_set_channel(5, vswTime.arg1);
    vofa_set_channel(6, vswTime.arg2);
    vofa_set_channel(7, vswTime.arg3);
    vofa_set_channel(8, vabc.arg1);
    vofa_set_channel(9, vabc.arg2);
    vofa_set_channel(10, vabc.arg3);
    vofa_send(1);
}

void sincos_test(void)
{
    static float angle = 0;
    angle += 0.1f;
    if (angle > 2 * PI) {
        angle -= 2 * PI;
    }
    float sin_value = arm_sin_f32(angle);
    float cos_value = arm_cos_f32(angle);
    vofa_set_channel(0, angle);
    vofa_set_channel(1, sin_value);
    vofa_set_channel(2, cos_value);
    vofa_send(1);
}

void set_duty(math_3f_t *swTime)
{
    TIM1->CCR1 = (uint32_t)(swTime->arg1 * PWM_PERIOD_CYCLES);
    TIM1->CCR2 = (uint32_t)(swTime->arg2 * PWM_PERIOD_CYCLES);
    TIM1->CCR3 = (uint32_t)(swTime->arg3 * PWM_PERIOD_CYCLES);
}