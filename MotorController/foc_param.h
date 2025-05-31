#ifndef __FOC_PARAM_H
#define __FOC_PARAM_H

#define FOC_TIMER_FREQ       (168000000) // 168MHz
#define FOC_TORQUE_LOOP_HZ   (20000)

#define FOC_TIM_PRESCALER    ((FOC_TIMER_FREQ / FOC_TORQUE_LOOP_HZ / 2) - 1)


#define FOC_DEAD_TIME       (200) // 200us


#define MOTOR_POLE_PAIRS 7              // pole pairs
#define ENCODER_CPR 16384               // encoder cpr

#endif 