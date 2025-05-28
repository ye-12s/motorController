#ifndef __VOFA_H
#define __VOFA_H

#include <stdint.h>

void vofa_init(void);
void vofa_set_channel(uint8_t channel, float value);
void vofa_send(uint32_t wait);

void vofa_send_test(float step);
int printk(const char *format, ...);

#endif // __VOFA_H
