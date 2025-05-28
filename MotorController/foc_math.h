//
// Created by An on 25-5-23.
//

#ifndef FOC_MATH_H
#define FOC_MATH_H

#include <stdint.h>

#include "foc_type.h"

void decompAlBe(float vecAmp, float angle, math_2f_t *talbe);
void clark(const math_3f_t *const tuvw, math_2f_t *talbe);
void park(const math_2f_t *const talbe, math_2f_t *tdq, float angle);
void invpark(const math_2f_t *const tdq, math_2f_t *talbe, float angle);
int svpwm(const math_2f_t *const ab, math_3f_t *swtiming, math_3f_t *duty);

float fastsqrt(float val);
float fastcos(float angle);
float fastsin(float angle);

#endif // FOC_MATH_H
