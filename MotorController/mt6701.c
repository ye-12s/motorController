#include "mt6701.h"
#include <math.h>

#include "stm32g4xx_hal.h"

#define MT6701_CS_LOW()  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET)
#define MT6701_CS_HIGH() HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;

static int mt6701_readRaw(uint8_t data)
{
    uint8_t reslut = 0;
    if (HAL_SPI_TransmitReceive(&hspi1, &data, &reslut, 1, 1) != HAL_OK) {
        return -1;
    }
    // SPI1->DR = 0xff;
    // while (!(SPI1->SR & SPI_SR_RXNE));
    return reslut;
}

static int mt6701_read(void)
{
    int temp[3] = {0};
    MT6701_CS_LOW();
    temp[0] = mt6701_readRaw(0x00);
    temp[1] = mt6701_readRaw(0x00);
    temp[2] = mt6701_readRaw(0x00);
    MT6701_CS_HIGH();
    if (temp[0] < 0 || temp[1] < 0 || temp[2] < 0) {
        return -1; // error 后续添加当前速度去计算这次的角度
    }
    uint32_t result = (temp[0] << 16) | (temp[1] << 8) | temp[2];
    return result;
}

int mt6701_sampleNow(encoder_t *enc)
{
    enc->raw           = mt6701_read();
    uint32_t angle_cnt = enc->raw >> 10;
    uint32_t diff      = angle_cnt - enc->offset;

    diff = fmod(diff, enc->cpr);

    enc->diff = diff;

    enc->rad = (float)diff * 2 * PI / enc->cpr;

    float dt_angle = enc->rad - enc->mech_rad_last;
    if (dt_angle > PI) {
        dt_angle -= 2 * PI;
    } else if (dt_angle < -PI) {
        dt_angle += 2 * PI;
    }
    enc->mech_rad_last      = enc->rad;
    float instantaneous_rpm = (dt_angle / (PI * 2)) * (60 / enc->sampledt);

    enc->rpm = instantaneous_rpm;

    enc->fittle_rpm = enc->alpha * instantaneous_rpm + (1 - enc->alpha) * enc->fittle_rpm;

    // float error = instantaneous_rpm - enc->pll_output;

    // enc->pll_integrator += error * enc->sampledt;
    // enc->pll_output += enc->pll_kp * error + enc->pll_ki * enc->pll_integrator;
    // enc->rpm = enc->pll_output;
    return 0;
}

int mt6701_calibOffset(encoder_t *enc)
{
    enc->raw    = mt6701_read();
    enc->offset = enc->raw >> 10;
    return 0;
}

float mt6701_getElecRad(encoder_t *enc)
{
    enc->elec_rad = enc->rad * enc->pp;
    enc->elec_rad = fmod(enc->elec_rad, 2 * PI);
    return enc->elec_rad;
}

float mt6701_gethpp(encoder_t *enc, float dt)
{
    float hpp = 0;

    return hpp;
}

float mt6701_getRpm(encoder_t *enc)
{
    return enc->rpm;
}
