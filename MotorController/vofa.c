#include "vofa.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32g4xx_hal.h"
#include "foc_math.h"
#include "usart.h"

// #define USING_VOFA_UART 1
#define USING_VOFA_USB_CDC 1

#define VOFA_CHANNEL_MAX   32

#define VOFA_SEND_END_FLAG (0x7F800000)

extern UART_HandleTypeDef huart1;

#if USING_VOFA_USB_CDC
#define VOFA_SEND_HANDLE(ptr, size, wait) \
    CDC_Transmit_FS((uint8_t *)ptr, size);
#elif USING_VOFA_UART
#define VOFA_SEND_HANDLE(ptr, size, wait) HAL_UART_Transmit_DMA(&huart3, ptr, size)
#else
#define VOFA_SEND_HANDLE(ptr, size, wait) (void)0
#endif //  USING_VOFA_UART

static float vofa_channel[VOFA_CHANNEL_MAX + 1] = {0};
static uint32_t size                            = 0;
void vofa_init(void)
{
    memset(vofa_channel, 0, sizeof(vofa_channel));
    size = 0;
#if USING_VOFA_UART

#endif
}

void vofa_set_channel(uint8_t channel, float value)
{
    if (channel > VOFA_CHANNEL_MAX || channel == 0) {
        return;
    }
    vofa_channel[channel] = value;
    size                  = channel + 1;
}

void vofa_send(uint32_t wait)
{
    if (size == 0) {
        return;
    }
    if (size + 1 > VOFA_CHANNEL_MAX) {
        size--; // 保证最后一个元素是结束标志
    }
    vofa_channel[0]                      = HAL_GetTick() / 1000.0f;
    (*(uint32_t *)&vofa_channel[size++]) = VOFA_SEND_END_FLAG;
    VOFA_SEND_HANDLE((uint8_t *)vofa_channel, size * sizeof(float), wait);
    size = 0;
}

// 输出正弦余弦波形
void vofa_send_test(float step)
{
    float sin_value    = 0;
    float cos_value    = 0;
    static float angle = 0;
    sin_value          = fastsin(angle * 3.1415926f / 180);
    cos_value          = fastcos(angle * 3.1415926f / 180);
    vofa_set_channel(0, sin_value);
    vofa_set_channel(1, cos_value);
    vofa_send(1);
    angle += step;
}

static char print_buffer[256];
int printk(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf(print_buffer, sizeof(print_buffer), format, args);
    va_end(args);

    HAL_UART_Transmit(&huart3, (uint8_t *)print_buffer, len, HAL_MAX_DELAY);
    return len;
}
