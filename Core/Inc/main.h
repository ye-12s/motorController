/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOC
#define APH4_Pin GPIO_PIN_3
#define APH4_GPIO_Port GPIOC
#define LIN1_Pin GPIO_PIN_13
#define LIN1_GPIO_Port GPIOB
#define LIN2_Pin GPIO_PIN_14
#define LIN2_GPIO_Port GPIOB
#define LIN3_Pin GPIO_PIN_15
#define LIN3_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_6
#define KEY4_GPIO_Port GPIOC
#define KEY3_Pin GPIO_PIN_7
#define KEY3_GPIO_Port GPIOC
#define KEY2_Pin GPIO_PIN_8
#define KEY2_GPIO_Port GPIOC
#define KEY1_Pin GPIO_PIN_9
#define KEY1_GPIO_Port GPIOC
#define HIN1_Pin GPIO_PIN_8
#define HIN1_GPIO_Port GPIOA
#define HIN2_Pin GPIO_PIN_9
#define HIN2_GPIO_Port GPIOA
#define HIN3_Pin GPIO_PIN_10
#define HIN3_GPIO_Port GPIOA
#define CSN_Pin GPIO_PIN_2
#define CSN_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
