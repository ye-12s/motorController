//
// Created by An on 25-5-23.
//

#ifndef FOC_TYPE_H
#define FOC_TYPE_H

#include <stdint.h>
#include <stdbool.h>

#define PWM_PERIOD_CYCLES   8400

#define SQRT3 1.7320508075688772935274463415059f
#define PI    3.14159265358979323846264338327950f

typedef struct {
    float arg1;
    float arg2;
} math_2f_t;

typedef struct {
    float arg1;
    float arg2;
    float arg3;
} math_3f_t;

typedef enum {
    S_IDLE = 0,       // 空闲
    S_IDLE_DONE,      // 空闲结束
    S_CHARG_BOOT_CAP, // 自举电容充电
    S_OFFICE_CALIB,   // 电流偏移校准
    S_ALIGN,          // 角度校准
    S_CLEAR,          // 清除参数
    S_START,          // 状态
    S_START_DONE,     // 状态结束
    S_RUN,            // 运行
    S_ANY_STOP,       // 任意模式跳转到停止
    S_STOP,           // 停止
    S_FAULT_NOW,      // 当前发生故障
    S_FAULT_OVER,     // 故障结束
} state_t;

typedef struct
{
    state_t state;
    int faultNow;
} statemachine_t;

typedef struct
{
    uint32_t raw;        // 原始值
    uint32_t offset;     // 0角度偏移值
    uint8_t pp;          // 电机极对数
    float rad;           // 电机转子角度 0~2pi
    float elec_rad;      // 电机转子电角度 0~2pi
    uint32_t cpr;        // 电机编码器线数
    int32_t diff;        // 与上次相比角度差
    float elec_rad_last; // 上次电角度
    float mech_rad_last; // 机械角度
    float sampledt;      // 采样时间
    float rpm;           // 电机转速
    float hpp;           // 电机转矩

    float fittle_rpm; // 滤波转速

    float alpha; // 滤波系数

    float pll_enable; // PLL使能
    float pll_ki;
    float pll_kp;
    float pll_integrator;

} encoder_t;

#endif // FOC_TYPE_H
