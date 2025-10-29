/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_G_Pin GPIO_PIN_14
#define LED_G_GPIO_Port GPIOC
#define LED_R_Pin GPIO_PIN_15
#define LED_R_GPIO_Port GPIOC
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOF
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOF
#define nAlert_Pin GPIO_PIN_0
#define nAlert_GPIO_Port GPIOB
#define nAlert_EXTI_IRQn EXTI0_IRQn
#define Power_EN1_Pin GPIO_PIN_1
#define Power_EN1_GPIO_Port GPIOB
#define Power_EN2_Pin GPIO_PIN_2
#define Power_EN2_GPIO_Port GPIOB
#define GPIO_EN_Pin GPIO_PIN_11
#define GPIO_EN_GPIO_Port GPIOB
#define USB3_BOOT_Pin GPIO_PIN_12
#define USB3_BOOT_GPIO_Port GPIOB
#define USB1_BOOT_Pin GPIO_PIN_13
#define USB1_BOOT_GPIO_Port GPIOB
#define USB2_BOOT_Pin GPIO_PIN_14
#define USB2_BOOT_GPIO_Port GPIOB
#define USB_RESET_Pin GPIO_PIN_15
#define USB_RESET_GPIO_Port GPIOB
#define Power_EN3_Pin GPIO_PIN_10
#define Power_EN3_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define ID1_GPIO_Port 		GLED_GPIO_Port
#define ID1_Pin 			GLED_Pin
#define ID2_GPIO_Port 		RLED_GPIO_Port
#define ID2_Pin 			RLED_Pin
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
