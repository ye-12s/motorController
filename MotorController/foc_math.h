//
// Created by An on 25-5-23.
//

#ifndef FOC_MATH_H
#define FOC_MATH_H

#include <stdint.h>

#include "foc_type.h"

#define DEG_TO_RAD(angle) ((angle) * 0.017453292519943295769236907684886f) // 180/PI
#define RAD_TO_DEG(angle) ((angle) * 57.295779513082320876798154814105f) // PI/180

void decompAlBe(float vecAmp, float angle, math_2f_t *talbe);
void clark(const math_3f_t *const tuvw, math_2f_t *talbe);
void park(const math_2f_t *const talbe, math_2f_t *tdq, float angle);
void invpark(const math_2f_t *const tdq, math_2f_t *talbe, float angle);
int svpwm(math_2f_t *talbe, math_3f_t *swtiming, float udc, int tpwm);
float pid_controller(pid_t *pid, float error);

void foc_math_test(void);
void foc_math_test_idq_const(void);
void sincos_test(void);

float fastsqrt(float val);
float fastcos(float angle);
float fastsin(float angle);

#endif // FOC_MATH_H
