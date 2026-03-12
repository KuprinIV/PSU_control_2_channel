/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EC1A_Pin GPIO_PIN_0
#define EC1A_GPIO_Port GPIOA
#define EC1B_Pin GPIO_PIN_1
#define EC1B_GPIO_Port GPIOA
#define EC1S_Pin GPIO_PIN_2
#define EC1S_GPIO_Port GPIOA
#define CC_CV_CH1_Pin GPIO_PIN_4
#define CC_CV_CH1_GPIO_Port GPIOA
#define DC_EN_CH1_Pin GPIO_PIN_5
#define DC_EN_CH1_GPIO_Port GPIOA
#define EC2A_Pin GPIO_PIN_6
#define EC2A_GPIO_Port GPIOA
#define EC2B_Pin GPIO_PIN_7
#define EC2B_GPIO_Port GPIOA
#define EC2S_Pin GPIO_PIN_4
#define EC2S_GPIO_Port GPIOC
#define BTN1_Pin GPIO_PIN_0
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin GPIO_PIN_1
#define BTN2_GPIO_Port GPIOB
#define CC_CV_CH2_Pin GPIO_PIN_12
#define CC_CV_CH2_GPIO_Port GPIOB
#define DC_EN_CH2_Pin GPIO_PIN_13
#define DC_EN_CH2_GPIO_Port GPIOB
#define BL_CTRL_Pin GPIO_PIN_14
#define BL_CTRL_GPIO_Port GPIOB
#define EC4A_Pin GPIO_PIN_6
#define EC4A_GPIO_Port GPIOC
#define EC4B_Pin GPIO_PIN_7
#define EC4B_GPIO_Port GPIOC
#define EC4S_Pin GPIO_PIN_8
#define EC4S_GPIO_Port GPIOC
#define LCD_RST_Pin GPIO_PIN_6
#define LCD_RST_GPIO_Port GPIOD
#define EC3S_Pin GPIO_PIN_5
#define EC3S_GPIO_Port GPIOB
#define EC3A_Pin GPIO_PIN_6
#define EC3A_GPIO_Port GPIOB
#define EC3B_Pin GPIO_PIN_7
#define EC3B_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
