//
// Created by An on 25-5-23.
//

#ifndef FOC_TYPE_H
#define FOC_TYPE_H

#include <stdint.h>
#include <stdbool.h>

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

typedef enum {
    FOC_MODE_OFF = 0, // 关机
    FOC_MODE_OPENLOOP, // 开环
    FOC_MODE_VF, // 电压频率控制
    FOC_MODE_IF, // 电流控制
}foc_mode_t;

typedef struct
{
    state_t state;
    int faultNow;
} statemachine_t;

#define SPEED_FILTER_LEN 8 

typedef struct
{
    uint8_t rx_buf[3];
    uint32_t raw;

    uint32_t offset;     // 0角度偏移值
    uint8_t pp;          // 电机极对数
    uint8_t dir;         // 电机方向 0:正转 1:反转
    float rad;           // 电机转子角度 0~2pi
    float elec_rad;      // 电机转子电角度 0~2pi
    uint32_t cpr;        // 电机编码器线数
    float diff;          // 与上次相比角度差
    float elec_rad_last; // 上次电角度
    float mech_rad_last; // 机械角度
    float sampledt;      // 采样时间
    
    float rpm;           // 电机转速
    float hpp;           // 电机转矩
    float omega;         // 电机角速度
    float fittle_rpm; // 滤波转速
    float alpha; // 滤波系数

    float angle_buffer[SPEED_FILTER_LEN];
    int buffer_idx;
    bool buffer_filled;

    uint8_t updated;
    uint32_t missed_cnt;
} encoder_t;

typedef struct
{
	float kp;
	float ki;
	float kd;
	float ka;   // 抗饱和系数
	float upper_limit;
	float lower_limit;
	float preErr;       // 前一次误差
	float integral;     // 积分
	float derivative;   // 微分
	float output;       // 输出
} pid_t;

typedef struct {
    int16_t rawADC[8];
    int16_t offsetADC[8]; // 偏移值
    int16_t calibCnt; 
    int32_t calibADC[4];

    uint32_t alignCnt;
    uint8_t alignTimes;

    uint32_t andle_div;
    uint32_t andle_div_cnt;

    foc_mode_t mode; // FOC模式

    float udc;
    float idc;
    
    math_3f_t swtime; // 开关时间
    math_3f_t Iabc_fb; // 电流反馈
    
    math_3f_t Uabc_fb; //反电动势 
    math_2f_t Ualbe_ref; 
    math_2f_t Udq_ref; 
    
    math_2f_t Idq_fb;
    math_2f_t Idq_ref;
    math_2f_t Ialbe_fb;

    float angle; 
    encoder_t encoder; // 编码器数据
    statemachine_t stateMachine;

    pid_t pid_id;       // 电流d轴PID
	pid_t pid_iq;       // 电流q轴PID
	pid_t pid_spd;      // 速度PID
	pid_t pid_pos;      // 位置PID

    float amp; // 电流幅值
}axit_t; 




#endif // FOC_TYPE_H
