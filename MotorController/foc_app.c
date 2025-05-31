#include "foc_app.h"
#include "adc.h"
#include "foc_math.h"
#include "foc_param.h"
#include "foc_type.h"
#include "mt6701.h"
#include "state_machine.h"
#include "stm32g4xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "vofa.h"


void foc_current_restruct(axit_t *axit);

float vangle = 0;
float vamp   = 0.8f;

axit_t g_axit = {
    .encoder = {
        .cpr = ENCODER_CPR,
        .pp  = MOTOR_POLE_PAIRS,
        .dir = 1,
    },
    .mode = FOC_MODE_VF,
    .udc  = 24.0f, // 电压
    .amp  = 0.8f,
};

void foc_currControl(axit_t *axit)
{
    clark(&axit->Iabc_fb, &axit->Ialbe_fb);
    park(&axit->Ialbe_fb, &axit->Idq_fb, axit->angle);

    // do PI control
    axit->Idq_ref.arg1 = 0.0f;      // Id reference
    axit->Idq_ref.arg2 = axit->amp; // Iq reference

    if (axit->mode == FOC_MODE_VF) {
        axit->Udq_ref = axit->Idq_ref;
    } else if (axit->mode == FOC_MODE_IF) {
        axit->Udq_ref.arg2 = pid_controller(&axit->pid_iq, axit->Idq_ref.arg2 - axit->Idq_fb.arg2);
        axit->Udq_ref.arg1 = 0; // Id control is not used in IF mode
    }

    invpark(&axit->Udq_ref, &axit->Ualbe_ref, axit->angle);
    svpwm(&axit->Ualbe_ref, &axit->swtime, axit->udc, FOC_TIM_PRESCALER);
    set_duty(&axit->swtime);
}

void foc_scheduler(axit_t *axit)
{
    foc_current_restruct(axit);
    state_t state = statemachine_getState(&axit->stateMachine);

    if (state != S_ALIGN) {
        mt6701_sampleNow(&axit->encoder);
        axit->angle = mt6701_getElecRad(&axit->encoder);
    }

    if (state == S_RUN) {
        foc_currControl(axit);
    } else if (state == S_OFFICE_CALIB) {
        if (axit->calibCnt < 500) {
            axit->calibADC[0] += axit->rawADC[0];
            axit->calibADC[1] += axit->rawADC[1];
            axit->calibADC[3] += axit->rawADC[3]; // Idc
            axit->calibCnt++;
        }
    }

    vofa_set_channel(1, axit->angle);
    vofa_set_channel(2, axit->Idq_fb.arg1);
    vofa_set_channel(3, axit->Idq_fb.arg2);
    vofa_set_channel(4, axit->Ialbe_fb.arg1);
    vofa_set_channel(5, axit->Ialbe_fb.arg2);
    vofa_set_channel(6, axit->Iabc_fb.arg1);
    vofa_set_channel(7, axit->Iabc_fb.arg2);
    vofa_set_channel(8, axit->Iabc_fb.arg3);
    vofa_send(1);
}

void stateMachine_task(axit_t *axit)
{
    int16_t status = 0;
    switch (axit->stateMachine.state) {
        case S_IDLE:
            break;
        case S_IDLE_DONE:
            pwm_OutputEnable();
            pwm_OutputLowSide();
            statemachine_nextState(&axit->stateMachine, S_CHARG_BOOT_CAP);
            break;
        case S_CHARG_BOOT_CAP:
            if (axit->calibCnt < 100) {
                axit->calibCnt++;
            } else {
                axit->calibCnt = 0;
                status         = 1;
            }
            if (status) {
                axit->calibCnt    = 0;
                axit->calibADC[0] = 0;
                axit->calibADC[1] = 0;
                statemachine_nextState(&axit->stateMachine, S_OFFICE_CALIB);
            }
            break;
        case S_OFFICE_CALIB:
            if (axit->calibCnt < 500) {

            } else {
                axit->offsetADC[0] = axit->calibADC[0] / (axit->calibCnt);
                axit->offsetADC[1] = axit->calibADC[1] / (axit->calibCnt);
                axit->offsetADC[3] = axit->calibADC[3] / (axit->calibCnt);
                status             = 1;
            }
            if (status) {
                statemachine_nextState(&axit->stateMachine, S_ALIGN);
            }
            break;
        case S_ALIGN:
            if (axit->alignTimes < 2) {
                if (axit->alignCnt < 2000) // 2s
                {
                    axit->Udq_ref.arg1 = 0.8f;
                    axit->Udq_ref.arg2 = 0.0f;
                    if (axit->alignTimes == 0) {
                        axit->angle = DEG_TO_RAD(270);
                    } else {
                        axit->angle = DEG_TO_RAD(0);
                    }
                    invpark(&axit->Udq_ref, &axit->Ualbe_ref, axit->angle);
                    svpwm(&axit->Ualbe_ref, &axit->swtime, axit->udc, FOC_TIM_PRESCALER);
                    set_duty(&axit->swtime);
                    axit->alignCnt++;
                } else {
                    // printk("align done tick now :0x%08u", HAL_GetTick());
                    if (mt6701_calibOffset(&axit->encoder) == 0) {
                        axit->alignCnt = 0;
                        axit->alignTimes++;
                    }
                }
            } else {
                status = 1;
            }
            if (status) {
                statemachine_nextState(&axit->stateMachine, S_CLEAR);
            }
            break;
        case S_CLEAR:
            axit->Idq_ref.arg1 = 0.0f;
            axit->Idq_ref.arg2 = 0.0f;
            axit->Udq_ref.arg1 = 0.0f;
            axit->Udq_ref.arg2 = 0.0f;
            axit->calibCnt     = 0;
            statemachine_nextState(&axit->stateMachine, S_START);
            break;
        case S_START:
            statemachine_nextState(&axit->stateMachine, S_START_DONE);
            break;
        case S_START_DONE:
            axit->Idq_ref.arg2 = axit->amp;
            statemachine_nextState(&axit->stateMachine, S_RUN);
            break;
        case S_RUN:

            break;
        case S_ANY_STOP:
            pwm_OutputDisable();
            statemachine_nextState(&axit->stateMachine, S_STOP);
            break;
        case S_STOP:
            statemachine_nextState(&axit->stateMachine, S_IDLE);
            break;
        case S_FAULT_NOW:
            pwm_OutputDisable();
            break;
        case S_FAULT_OVER:
            if (status) {
            } else {
                statemachine_nextState(&axit->stateMachine, S_IDLE);
            }
            break;
    }
}

void foc_app_init(void)
{
    // 初始化状态机
    vofa_init();
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, __HAL_TIM_GET_AUTORELOAD(&htim1) - 44);
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart(&hadc2);

    pwm_OutputDisable();
    pwm_OutputLowSide();

    statemachine_init(&g_axit.stateMachine);

    // 初始化VOFA
    vofa_init();

    // pwm_OutputEnable();
    // statemachine_nextState(&g_axit.stateMachine, S_IDLE_DONE);
    g_axit.udc = 24.0f; // 设置电压
}


void openloop_mode(void)
{
    math_2f_t valbe   = {0};
    math_3f_t vswTime = {0};
    vangle += 0.05f;
    if (vangle > 2 * PI) {
        vangle -= 2 * PI;
    }

    decompAlBe(vamp, vangle, &valbe);
    int sector = svpwm(&valbe, &vswTime, 24.0f, FOC_TIM_PRESCALER);
    set_duty(&vswTime);
}

void set_duty(math_3f_t *swTime)
{
    TIM1->CCR1 = (uint32_t)(swTime->arg1);
    TIM1->CCR2 = (uint32_t)(swTime->arg2);
    TIM1->CCR3 = (uint32_t)(swTime->arg3);
}

// 导通MOS下管
void pwm_OutputLowSide(void)
{
    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;
}

void pwm_OutputDisable(void)
{
    // TIM1->CCR1 = 0;
    // TIM1->CCR2 = 0;
    // TIM1->CCR3 = 0;
    // CLEAR_BIT(TIM1->BDTR, TIM_BDTR_MOE);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
}

void pwm_OutputEnable(void)
{
    // SET_BIT(TIM1->BDTR, TIM_BDTR_MOE);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, __HAL_TIM_GET_AUTORELOAD(&htim1) - 44);
}

void foc_current_restruct(axit_t *axit)
{
    axit->rawADC[0] = ADC1->JDR1; // Ia
    axit->rawADC[1] = ADC1->JDR2; // Ib
    axit->rawADC[2] = ADC1->JDR3; // Ic
    axit->rawADC[3] = ADC1->JDR4; // Idc
    axit->rawADC[4] = ADC2->JDR1; // udc
    axit->rawADC[5] = ADC2->JDR2; // ua
    axit->rawADC[6] = ADC2->JDR3; // ub
    axit->rawADC[7] = ADC2->JDR4; // uc

    // 电流还原
    axit->Iabc_fb.arg1 = (float)(axit->rawADC[0] - axit->offsetADC[0]) * 3.3f / 4096.0f / (0.005 * 20); // Ia
    axit->Iabc_fb.arg2 = (float)(axit->rawADC[1] - axit->offsetADC[1]) * 3.3f / 4096.0f / (0.005 * 20); // Ib
    axit->Iabc_fb.arg3 = -axit->Iabc_fb.arg1 - axit->Iabc_fb.arg2;                                      // Ic
    axit->idc          = (float)(axit->rawADC[3] - axit->offsetADC[3]) * 3.3f / 4096.0f / (0.005 * 20); // Idc

    // 电压还原
    axit->Uabc_fb.arg1 = (float)axit->rawADC[5] * 3.3f / 4096.0f * 16; // ua
    axit->Uabc_fb.arg2 = (float)axit->rawADC[6] * 3.3f / 4096.0f * 16; // ub
    axit->Uabc_fb.arg3 = (float)axit->rawADC[7] * 3.3f / 4096.0f * 16; // uc
    axit->udc          = (float)axit->rawADC[4] * 3.3f / 4096.0f * 16; // udc
}



// 电流采样中断
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        foc_scheduler(&g_axit);
    }
}
