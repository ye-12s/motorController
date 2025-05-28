#ifndef __MC_MT6701_H__
#define __MC_MT6701_H__

#include "foc_type.h"

float mt6701_getElecRad(encoder_t *enc);
int mt6701_calibOffset(encoder_t *enc);
int mt6701_sampleNow(encoder_t *enc);
float mt6701_getRpm(encoder_t *enc);

#endif
