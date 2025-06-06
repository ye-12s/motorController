//
// Created by An on 25-5-23.
//

#include "foc_math.h"
#include "arm_math.h"
#include "foc_param.h"
#include "foc_type.h"
#include "vofa.h"
#define HOLLYST 0.017453292519943295769236907684886f

const float pfSinTable[] = {
    0.0,                                // sin(0)
    0.17364817766693034885171662676931, // sin(10)
    0.34202014332566873304409961468226, // sin(20)
    0.5,                                // sin(30)
    0.64278760968653932632264340990726, // sin(40)
    0.76604444311897803520239265055542, // sin(50)
    0.86602540378443864676372317075294, // sin(60)
    0.93969262078590838405410927732473, // sin(70)
    0.98480775301220805936674302458952, // sin(80)
    1.0                                 // sin(90)
};

const float pfCosTable[] = {
    1.0,                                // cos(0)
    0.99984769515639123915701155881391, // cos(1)
    0.99939082701909573000624344004393, // cos(2)
    0.99862953475457387378449205843944, // cos(3)
    0.99756405025982424761316268064426, // cos(4)
    0.99619469809174553229501040247389, // cos(5)
    0.99452189536827333692269194498057, // cos(6)
    0.99254615164132203498006158933058, // cos(7)
    0.99026806874157031508377486734485, // cos(8)
    0.98768834059513772619004024769344  // cos(9)
};

void decompAlBe(float vecAmp, float angle, math_2f_t *talbe)
{
    talbe->arg1 = vecAmp * arm_cos_f32(angle);
    talbe->arg2 = vecAmp * arm_sin_f32(angle);
}

void decompDcbus(float busAct, math_2f_t *talbe, math_2f_t *talbecomp)
{
    talbecomp->arg1 = 1.0f / busAct * talbe->arg1;
    talbecomp->arg2 = 1.0f / busAct * talbe->arg2;

    if (talbecomp->arg1 > 1.0f) {
        talbecomp->arg1 = 1.0f;
    } else if (talbecomp->arg1 < -1.0f) {
        talbecomp->arg1 = -1.0f;
    }

    if (talbecomp->arg2 > 1.0f) {
        talbecomp->arg2 = 1.0f;
    } else if (talbecomp->arg2 < -1.0f) {
        talbecomp->arg2 = -1.0f;
    }
}

void clark(const math_3f_t *const tuvw, math_2f_t *talbe)
{
    talbe->arg1 = tuvw->arg1;
    talbe->arg2 = (1.0f / SQRT3) * (tuvw->arg2 - tuvw->arg3);
}
void park(const math_2f_t *const talbe, math_2f_t *tdq, float angle)
{
    float fsin = arm_sin_f32(angle);
    float fcos = arm_cos_f32(angle);

    tdq->arg1 = fcos * talbe->arg1 + fsin * talbe->arg2;
    tdq->arg2 = fcos * talbe->arg2 - fsin * talbe->arg1;
}
void invpark(const math_2f_t *const tdq, math_2f_t *talbe, float angle)
{
    float fsin = arm_sin_f32(angle);
    float fcos = arm_cos_f32(angle);

    talbe->arg1 = tdq->arg1 * fcos - tdq->arg2 * fsin;
    talbe->arg2 = tdq->arg1 * fsin + tdq->arg2 * fcos;
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

void foc_math_test(void)
{
    static float vangle = 0.0f; // 角度变量
    math_2f_t Idq       = {0};
    math_2f_t Ialbe     = {0};
    math_3f_t swtime    = {0};
    math_2f_t Udq       = {0};
    math_2f_t Ualbe     = {0};
    math_3f_t Iabc_fb   = {0};
    vangle += 0.01f;
    vangle = fmodf(vangle, 2 * PI);
    if (vangle > 2 * PI) {
        vangle -= 2 * PI;
    }
    // 测试 Clarke 变换
    Iabc_fb.arg1 = 1.0f;
    Iabc_fb.arg2 = -0.5f;
    Iabc_fb.arg3 = -0.5f;
    clark(&Iabc_fb, &Ialbe);

    // 测试 Park 变换
    float test_angle = vangle;
    park(&Ialbe, &Idq, test_angle);

    // 反Park变换
    invpark(&Idq, &Ualbe, test_angle);

    // SVPWM 测试
    svpwm(&Ualbe, &swtime, 24.0f, FOC_TIM_PRESCALER);

    // 发送测试数据到 vofa

    vofa_set_channel(1, vangle);
    vofa_set_channel(2, Idq.arg2);
    vofa_set_channel(3, Idq.arg1);
    vofa_set_channel(4, Ialbe.arg1);
    vofa_set_channel(5, Ialbe.arg2);
    vofa_set_channel(6, Ualbe.arg1);
    vofa_set_channel(7, Ualbe.arg2);
    vofa_set_channel(8, swtime.arg1);
    vofa_set_channel(9, swtime.arg2);
    vofa_set_channel(10, swtime.arg3);

    vofa_send(1);
}

void foc_math_test_idq_const(void)
{
    static float vangle = 0.0f; // 角度变量
    math_2f_t Idq       = {0};
    math_2f_t Ialbe     = {0};
    math_3f_t swtime    = {0};
    math_2f_t Udq       = {0};
    math_2f_t Ualbe     = {0};

    vangle += 0.01f;
    vangle = fmodf(vangle, 2 * PI);
    if (vangle > 2 * PI) {
        vangle -= 2 * PI;
    }

    // 让Ialbe随角度变化
    Ialbe.arg1 = arm_cos_f32(vangle);
    Ialbe.arg2 = arm_sin_f32(vangle);

    // Park变换
    float test_angle = vangle;
    park(&Ialbe, &Idq, test_angle);

    // 反Park变换
    invpark(&Idq, &Ualbe, test_angle);

    // SVPWM 测试
    svpwm(&Ualbe, &swtime, 24.0f, FOC_TIM_PRESCALER);

    // 发送测试数据到 vofa
    vofa_set_channel(1, vangle);
    vofa_set_channel(2, Idq.arg2);
    vofa_set_channel(3, Idq.arg1);
    vofa_set_channel(4, Ialbe.arg1);
    vofa_set_channel(5, Ialbe.arg2);
    vofa_set_channel(6, Ualbe.arg1);
    vofa_set_channel(7, Ualbe.arg2);
    vofa_set_channel(8, swtime.arg1);
    vofa_set_channel(9, swtime.arg2);
    vofa_set_channel(10, swtime.arg3);

    vofa_send(1);
}

#if 0

int svpwm(const math_2f_t *const ab, math_3f_t *swtiming, math_3f_t *duty)
{
    int sector = 0;

    float temp1 = ab->arg2;
    float temp2 = (SQRT3 / 2.0f) * ab->arg1 - 0.5f * ab->arg2;
    float temp3 = (-SQRT3 / 2.0f) * ab->arg1 - 0.5f * ab->arg2;

    int A = temp1 > 0 ? 1 : 0;
    int B = temp2 > 0 ? 1 : 0;
    int C = temp3 > 0 ? 1 : 0;

    int N = (C << 2) | (B << 1) | A;

    temp1 = ab->arg2;                              // x
    temp2 = 0.5f * (SQRT3 * ab->arg1 + ab->arg2);  // y
    temp3 = 0.5f * (-SQRT3 * ab->arg1 + ab->arg2); // z

    float T4, T6;

    switch (N) {
        case 1:
            T4     = temp3;
            T6     = temp2;
            sector = 2;
            break;
        case 2:
            T4     = temp2;
            T6     = -temp1;
            sector = 6;
            break;
        case 3:
            T4     = -temp3;
            T6     = temp1;
            sector = 1;
            break;
        case 4:
            T4     = -temp1;
            T6     = temp3;
            sector = 4;
            break;
        case 5:
            T4     = temp1;
            T6     = -temp2;
            sector = 3;
            break;
        case 6:
            T4     = -temp2;
            T6     = -temp3;
            sector = 5;
            break;
    }

    float Ta = (1.0f - T4 - T6) / 4.0f;
    float Tb = Ta + T4 / 2.0f;
    float Tc = Tb + T6 / 2.0f;

    switch (N) {
        case 1:
            swtiming->arg1 = Tb;
            swtiming->arg2 = Ta;
            swtiming->arg3 = Tc;
            break;
        case 2:
            swtiming->arg1 = Ta;
            swtiming->arg2 = Tc;
            swtiming->arg3 = Tb;
            break;
        case 3:
            swtiming->arg1 = Ta;
            swtiming->arg2 = Tb;
            swtiming->arg3 = Tc;
            break;
        case 4:
            swtiming->arg1 = Tc;
            swtiming->arg2 = Tb;
            swtiming->arg3 = Ta;
            break;
        case 5:
            swtiming->arg1 = Tc;
            swtiming->arg2 = Ta;
            swtiming->arg3 = Tb;
            break;
        case 6:
            swtiming->arg1 = Tb;
            swtiming->arg2 = Tc;
            swtiming->arg3 = Ta;
            break;
    }

    sector = N;

    duty->arg1 = 1.0f - 2.0f * swtiming->arg1;
    duty->arg2 = 1.0f - 2.0f * swtiming->arg2;
    duty->arg3 = 1.0f - 2.0f * swtiming->arg3;

    return sector;
}

#endif

int svpwm(math_2f_t *talbe, math_3f_t *swtiming, float udc, int tpwm)
{
    int sector = 0;
    float Ta, Tb, Tc;
    float Tx, Ty;
    if (talbe->arg2 > 0) {
        sector += 1;
    }
    if ((SQRT3 * talbe->arg1 - talbe->arg2) / 2.0f > 0) {
        sector += 2;
    }
    if ((-SQRT3 * talbe->arg1 - talbe->arg2) / 2.0f > 0) {
        sector += 4;
    }

    switch (sector) {
        case 1:
            Tx = (-1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc);
            Ty = (1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc);
            break;
        case 2:
            Tx = (1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc);
            Ty = -(1.73205078F * talbe->arg2 * tpwm / udc);
            break;
        case 3:
            Tx = -((-1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc));
            Ty = 1.73205078F * talbe->arg2 * tpwm / udc;
            break;
        case 4:
            Tx = -(1.73205078F * talbe->arg2 * tpwm / udc);
            Ty = (-1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc);
            break;
        case 5:
            Tx = 1.73205078F * talbe->arg2 * tpwm / udc;
            Ty = -((1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc));
            break;
        default:
            Tx = -((1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc));
            Ty = -((-1.5F * talbe->arg1 + 0.866025388F * talbe->arg2) * (tpwm / udc));
            break;
    }

    float f_temp = Tx + Ty;
    if (f_temp > tpwm) {
        Tx /= f_temp;
        Ty /= (Tx + Ty);
    }
    Ta = (tpwm - (Tx + Ty)) / 4.0F;
    Tb = Tx / 2.0F + Ta;
    Tc = Ty / 2.0F + Tb;
    switch (sector) {
        case 1:
            swtiming->arg1 = Tb;
            swtiming->arg2 = Ta;
            swtiming->arg3 = Tc;
            break;

        case 2:
            swtiming->arg1 = Ta;
            swtiming->arg2 = Tc;
            swtiming->arg3 = Tb;
            break;

        case 3:
            swtiming->arg1 = Ta;
            swtiming->arg2 = Tb;
            swtiming->arg3 = Tc;
            break;

        case 4:
            swtiming->arg1 = Tc;
            swtiming->arg2 = Tb;
            swtiming->arg3 = Ta;
            break;

        case 5:
            swtiming->arg1 = Tc;
            swtiming->arg2 = Ta;
            swtiming->arg3 = Tb;
            break;

        case 6:
            swtiming->arg1 = Tb;
            swtiming->arg2 = Tc;
            swtiming->arg3 = Ta;
            break;
    }

    return sector;
}

float pid_controller(pid_t *pid, float error)
{
    float kp_term = pid->kp * error;

    pid->integral += error;
    float ki_term = pid->ki * pid->integral;

    pid->output = kp_term + ki_term;

    if (pid->output > pid->upper_limit) {
        float sat = pid->upper_limit - pid->output;
        pid->integral += sat * pid->ka;
        pid->output = pid->upper_limit;
    } else if (pid->output < pid->lower_limit) {
        float sat = pid->lower_limit - pid->output;
        pid->integral += sat * pid->ka;
        pid->output = pid->lower_limit;
    }

    return pid->output;
}

float fastsin(float angle)
{
    int sig        = 0;
    float tmpAngle = 0;
    if (angle >= 0.5f) {
        sig   = 1;
        angle = angle - 0.5f;
    }
    tmpAngle = (angle > 0.25f) ? (0.5f - angle) : angle;

    int a     = tmpAngle * 36.0f;
    float b   = tmpAngle * 360.0f - a * 10.0f;
    float sin = pfSinTable[a] * pfCosTable[(int)b] + b * HOLLYST * pfSinTable[9 - a];
    if (sin > 1.0f) {
        sin = 1.0f;
    }
    return (sig > 0) ? -sin : sin;
}

float fastcos(float angle)
{
    int sig        = 0;
    float tmpAngle = 0;

    if (angle <= 0.75f) {
        angle += 0.25f;
    } else {
        angle -= 0.75f;
    }

    if (angle >= 0.5f) {
        sig   = 1;
        angle = angle - 0.5f;
    }
    tmpAngle = (angle > 0.25f) ? (0.5f - angle) : angle;

    int a     = tmpAngle * 36.0f;
    float b   = tmpAngle * 360.0f - a * 10.0f;
    float sin = pfSinTable[a] * pfCosTable[(int)b] + b * HOLLYST * pfSinTable[9 - a];
    if (sin > 1.0f) {
        sin = 1.0f;
    }
    return (sig > 0) ? -sin : sin;
}

float fastsqrt(float val)
{
    long i;
    float x, y;
    const float f = 1.5F;

    x = val * 0.5F;
    y = val;
    i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (f - (x * y * y));
    y = y * (f - (x * y * y));
    return val * y;
}
