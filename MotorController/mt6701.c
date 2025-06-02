#include "mt6701.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "foc_type.h"
#include "stm32g4xx_hal.h"

#define MT6701_CS_LOW()  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET)
#define MT6701_CS_HIGH() HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;

uint8_t crc6_itu(uint8_t *data, uint32_t length)
{
    uint8_t i;
    uint8_t crc = 0; // Initial value
    while (length--) {
        crc ^= *data++; // crc ^= *data; data++;
        for (i = 6; i > 0; --i) {
            if (crc & 0x20)
                crc = (crc << 1) ^ 0x03;
            else
                crc = (crc << 1);
        }
    }
    return (crc & 0x3f);
}

static int mt6701_readRaw(uint8_t data)
{
    uint8_t reslut = 0;
    if (HAL_SPI_TransmitReceive(&hspi1, &data, &reslut, 1, 1) != HAL_OK) {
        return -1;
    }
    return reslut;
}
// 0x0034F40B
static int mt6701_read(void)
{
    uint8_t temp[3] = {0};
    MT6701_CS_LOW();
    if (HAL_SPI_TransmitReceive(&hspi1, &temp, &temp, 3, 1) != HAL_OK) {
        return -1;
    }
    MT6701_CS_HIGH();
    if (temp[0] < 0 || temp[1] < 0 || temp[2] < 0) {
        return -1; // error 后续添加当前速度去计算这次的角度
    }
    uint32_t result     = (temp[0] << 16) | (temp[1] << 8) | temp[2];
    uint8_t crc_temp[3] = {
        result >> 18,
        result >> 12,
        result >> 6};
    uint8_t cala_crc = crc6_itu(crc_temp, 3);
    uint8_t crc      = temp[2] & 0x3f; // 取低6位

    if (cala_crc != crc) {
        return -2; // CRC error
    }
    return result;
}

int mt6701_sampleNow(encoder_t *enc)
{
    if (enc->updated == true) {
        return -1; // Encoder already updated, cannot sample now
    }
    MT6701_CS_LOW();
    HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t[]){0x00, 0x00, 0x00}, (uint8_t *)enc->rx_buf, 3);
    return 0;
}

int mt6701_calibOffset(encoder_t *enc)
{
    if (enc->updated == false) {
        return -1; // Encoder not updated, cannot calibrate offset
    }
    uint32_t raw     = enc->rx_buf[0] << 16 | enc->rx_buf[1] << 8 | enc->rx_buf[2];
    uint8_t calc_crc = crc6_itu((uint8_t[]){raw >> 18, raw >> 12, raw >> 6}, 3);
    uint8_t crc      = raw & 0x3f; // 取低6位
    if (calc_crc != crc) {
        return -2; // CRC error
    }
    enc->offset = raw >> 10;
    return 0;
}

float mt6701_getElecRad(encoder_t *enc)
{
    enc->elec_rad = enc->rad * enc->pp;
    enc->elec_rad = fmodf(enc->elec_rad, 2 * PI);
    return enc->elec_rad;
}

float mt6701_gethpp(encoder_t *enc, float dt)
{
    float hpp = 0;
    if (enc->rpm > 0) {
        hpp = enc->fittle_rpm * enc->pp * 2 * PI / 60.0f; // 转矩
    } else if (enc->rpm < 0) {
        hpp = -enc->fittle_rpm * enc->pp * 2 * PI / 60.0f; // 转矩
    }
    return hpp;
}

float mt6701_getRpm(encoder_t *enc)
{
    return enc->fittle_rpm;
}
float normalize_angle(float rad)
{
    rad = fmodf(rad, 2 * PI);
    // if (rad > PI) {
    //     rad -= 2 * PI;
    // } else if (rad < -PI) {
    //     rad += 2 * PI;
    // }

    if (rad < 0) {
        rad += 2 * PI; // 确保角度在0到2π之间
    }

    return rad;
}
void mt6701_update(encoder_t *enc)
{
#if 0
    float dt = enc->sampledt;

    if (enc->updated == 0) {
        enc->missed_cnt++;

        float est_rad = enc->rad + enc->fittle_rpm * dt * (2 * PI / 60.0f);
        est_rad       = normalize_angle(est_rad);
        if (est_rad < 0) est_rad += 2 * PI;
        enc->rad = est_rad;
    } else {
        enc->updated = 0;

        int32_t angle_cnt = enc->raw - enc->offset;
        if (enc->dir == 1) {
            angle_cnt = -angle_cnt;
        }

        angle_cnt = fmodf(angle_cnt, enc->cpr);
        if (angle_cnt < 0) {
            angle_cnt += enc->cpr;
        }

        float new_rad = (float)angle_cnt * 2 * PI / enc->cpr;
        new_rad       = normalize_angle(new_rad);

        enc->rad = new_rad;

        // 滑动窗口法计算 omega（速度）
        enc->angle_buffer[enc->buffer_idx] = new_rad;
        int oldest_idx                     = (enc->buffer_idx + 1) % SPEED_FILTER_LEN;
        enc->buffer_idx                    = (enc->buffer_idx + 1) % SPEED_FILTER_LEN;

        float angle_diff = new_rad - enc->angle_buffer[oldest_idx];
        if (angle_diff > PI) angle_diff -= 2 * PI;
        if (angle_diff < -PI) angle_diff += 2 * PI;

        float total_dt = dt * (enc->buffer_filled ? SPEED_FILTER_LEN : enc->buffer_idx);
        if (total_dt > 0) {
            enc->omega              = angle_diff / total_dt;
            float instantaneous_rpm = (angle_diff / (2 * PI)) * (60.0f / total_dt);
            enc->rpm                = instantaneous_rpm;
            enc->fittle_rpm         = enc->alpha * instantaneous_rpm + (1 - enc->alpha) * enc->fittle_rpm;
        }

        enc->mech_rad_last = new_rad;
        if (enc->buffer_idx == 0) enc->buffer_filled = true;

        enc->missed_cnt = 0;
    }
#else
    if (enc->updated == 0) {
        enc->missed_cnt++;

        // 使用滤波后的速度预测角度
        float dt       = enc->sampledt;
        float last_rad = enc->rad;
        float est_rad  = last_rad + enc->fittle_rpm * dt * (2 * PI / 60.0f);
        est_rad        = normalize_angle(est_rad);

        if (est_rad < 0) {
            est_rad += 2 * PI;
        }

        enc->rad = est_rad;
    } else {
        enc->updated = 0;

        float angle_cnt = enc->raw - enc->offset;
        if (enc->dir == 1) {
            angle_cnt = -angle_cnt;
        }

        angle_cnt = fmodf(angle_cnt, enc->cpr);
        if (angle_cnt < 0) {
            angle_cnt += enc->cpr;
        }

        enc->rad  = (float)angle_cnt * 2 * PI / enc->cpr;
        enc->rad  = normalize_angle(enc->rad);
        enc->diff = enc->rad - enc->mech_rad_last;

        // 防止角度突变
        if (enc->diff > PI) {
            enc->diff -= 2 * PI;
        } else if (enc->diff < -PI) {
            enc->diff += 2 * PI;
        }

        // // 限幅角度差值，避免瞬时跳变
        // float max_rad_step = 2 * PI * 0.5f; // 每周期最多跳动0.5圈（可调）
        // if (enc->diff > max_rad_step) enc->diff = max_rad_step;
        // if (enc->diff < -max_rad_step) enc->diff = -max_rad_step;

        enc->mech_rad_last = enc->rad;

        float dt                = enc->sampledt * (enc->missed_cnt + 1);
        enc->omega              = enc->diff / dt;
        float instantaneous_rpm = (enc->diff / (2 * PI)) * (60.0f / dt);
        enc->rpm                = instantaneous_rpm;

        // 使用低通滤波后的速度
        enc->fittle_rpm = enc->alpha * instantaneous_rpm + (1.0f - enc->alpha) * enc->fittle_rpm;

        enc->missed_cnt = 0;
    }
#endif
}

void mt6701_spi_cb(encoder_t *enc)
{
    MT6701_CS_HIGH();
    uint32_t raw = enc->rx_buf[0] << 16 | enc->rx_buf[1] << 8 | enc->rx_buf[2];
    uint8_t crc  = raw & 0x3f; // 取低6位

    uint8_t calc_crc = crc6_itu((uint8_t[]){raw >> 18, raw >> 12, raw >> 6}, 3); // 计算CRC
    if (crc != calc_crc) {
        return;
    }
    enc->raw     = raw >> 10;
    enc->updated = 1; // 更新标志位
}

int mt6701_init(encoder_t *enc)
{
    enc->buffer_idx    = 0;
    enc->buffer_filled = false;
    for (int i = 0; i < SPEED_FILTER_LEN; i++) {
        enc->angle_buffer[i] = 0.0f;
    }
}
