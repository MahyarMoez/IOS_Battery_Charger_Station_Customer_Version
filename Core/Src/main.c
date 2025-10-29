/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "led_animation.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include "app_version.h"

// STM PD
#include "stusb4500.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
__attribute__((used, section(".fw_header"), aligned(4)))
const char fw_header[] = "APP_VERSION" APP_VERSION "\n";
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim3_ch2;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM3_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
enum INTERRUPT_STATUS {
	DEVICE_STATUS_IDLE = 							0x00,
	USB_STATUS_JUST_DISCONNECTED = 					0x01,
	USB_STATUS_CONNECTED = 							0x10,
	USB_STATUS_JUST_CONNECTED = 					0x11,
	BATTERY_STATUS_CHARGING = 						0x20,
	BATTERY_STATUS_CHARGING_STARTED = 				0x21,
	BATTERY_STATUS_CHARGING_TERMINATED = 			0x22,

	USB_STATUS_BAD_CHARGER_ATTACHED =				0x30,
	USB_STATUS_BAD_CHARGER_MID_POWER_ATTACHED =		0x31,
	USB_STATUS_BAD_CHARGER_LOW_POWER_ATTACHED =		0x32,
	USB_STATUS_BAD_CHARGER = 						0x34,
};
static volatile uint8_t CurrentDeviceStatus = DEVICE_STATUS_IDLE;

__attribute__((section(".noinit"))) volatile uint32_t MagicFlag;
const uint32_t DFU_FLAG_VALUE = 0xDEADBEEFUL;

static void CheckCurrentUSBPD(void)
{
	uint8_t total_charging_ports = 0;

	HAL_StatusTypeDef status = STUSB4500_ReadCurrentPDO(&total_charging_ports);
	if(status != HAL_OK) {
		if(status == HAL_ERROR) {
			Error_Handler();
		}
		return;
	}

	switch(total_charging_ports) {
		case 0:
		{
			  HAL_GPIO_WritePin(GPIOB, Power_EN1_Pin|Power_EN2_Pin, GPIO_PIN_SET);
			  HAL_GPIO_WritePin(Power_EN3_GPIO_Port, Power_EN3_Pin, GPIO_PIN_SET);
			if(CurrentDeviceStatus != USB_STATUS_BAD_CHARGER) {
				CurrentDeviceStatus = USB_STATUS_BAD_CHARGER_ATTACHED;
			}
		}
		break;
		case 1:
		{
			  HAL_GPIO_WritePin(GPIOB, Power_EN1_Pin|Power_EN2_Pin, GPIO_PIN_SET);
			  HAL_GPIO_WritePin(Power_EN3_GPIO_Port, Power_EN3_Pin, GPIO_PIN_RESET);
   			  CurrentDeviceStatus = USB_STATUS_BAD_CHARGER_LOW_POWER_ATTACHED;
		}
		break;
		case 2:
		{
			  HAL_GPIO_WritePin(GPIOB, Power_EN1_Pin, GPIO_PIN_SET);
			  HAL_GPIO_WritePin(GPIOB, Power_EN2_Pin, GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(Power_EN3_GPIO_Port, Power_EN3_Pin, GPIO_PIN_RESET);
  			  CurrentDeviceStatus = USB_STATUS_BAD_CHARGER_MID_POWER_ATTACHED;
		}
		break;
		case 3:
		default:
		{
			  HAL_GPIO_WritePin(GPIOB, Power_EN1_Pin|Power_EN2_Pin, GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(Power_EN3_GPIO_Port, Power_EN3_Pin, GPIO_PIN_RESET);
			if(CurrentDeviceStatus == USB_STATUS_BAD_CHARGER) {
				CurrentDeviceStatus = USB_STATUS_JUST_DISCONNECTED;
			}
		}
		break;
	}
}

static void EnterDFUMode_Reset(void)
{
    __set_PRIMASK(1);  // Disable interrupts
    NVIC_SystemReset();
}

static void EnterDFUMode()
{
	if (MagicFlag != DFU_FLAG_VALUE)
	{
		return;
	}

	MagicFlag = 0;  // Clear flag

    void (*boot_jump)(void);
    uint32_t boot_address = 0x1FFF0000;  // System memory bootloader address

    __disable_irq();  // Disable all interrupts
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // Remap system memory to 0x00000000
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    // Set stack pointer to system memory
    __set_MSP(*(volatile uint32_t*)boot_address);

    // Set function pointer to reset vector
    boot_jump = (void (*)(void)) (*(volatile uint32_t*) (boot_address + 4));

    // Jump to system memory bootloader
    boot_jump();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  EnterDFUMode();
  MagicFlag = 0;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_USB_PCD_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  init_main_led_animation();
  led_animation_main();

  HAL_Delay(1000);  // wait for device restart

  bool is_ok = false;
  HAL_StatusTypeDef status = STUSB4500_ReadBackPDOs(&is_ok);
  if(HAL_OK == status) {
	  if(is_ok == false) {
		  HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_SET);
		  if(HAL_OK != STUSB4500_Initialize()) {
			  Error_Handler();
		  }
		  HAL_GPIO_WritePin(GPIOC, LED_R_Pin, GPIO_PIN_RESET);
	  }
  }
  else {
	  Error_Handler();
  }

  CheckCurrentUSBPD();


  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 15;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x30A175AB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 154;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */
	hpcd_USB_FS.pData = &hUsbDeviceFS;
  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */
	if(USBD_Init(&hUsbDeviceFS, &Class_Desc, 0) != USBD_OK)
	{
		Error_Handler();
	}

	if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
	{
		Error_Handler();
	}

	if(USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_CDC_Template_fops) != USBD_OK)
	{
		Error_Handler();
	}

	if(USBD_Start(&hUsbDeviceFS) != USBD_OK)
	{
		Error_Handler();
	}
  /* USER CODE END USB_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_G_Pin|LED_R_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Power_EN1_Pin|Power_EN2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_RESET_GPIO_Port, USB_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Power_EN3_GPIO_Port, Power_EN3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_G_Pin LED_R_Pin */
  GPIO_InitStruct.Pin = LED_G_Pin|LED_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : nAlert_Pin */
  GPIO_InitStruct.Pin = nAlert_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(nAlert_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Power_EN1_Pin Power_EN2_Pin USB_RESET_Pin */
  GPIO_InitStruct.Pin = Power_EN1_Pin|Power_EN2_Pin|USB_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_EN_Pin */
  GPIO_InitStruct.Pin = GPIO_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB3_BOOT_Pin USB1_BOOT_Pin USB2_BOOT_Pin */
  GPIO_InitStruct.Pin = USB3_BOOT_Pin|USB1_BOOT_Pin|USB2_BOOT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Power_EN3_Pin */
  GPIO_InitStruct.Pin = Power_EN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Power_EN3_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_2);

	switch(CurrentDeviceStatus)
	{
	case USB_STATUS_BAD_CHARGER_ATTACHED:
	{
		CurrentDeviceStatus = USB_STATUS_BAD_CHARGER;
		init_led_animation_power_error();
	}
		break;
	case USB_STATUS_BAD_CHARGER_MID_POWER_ATTACHED:
	{
		CurrentDeviceStatus = USB_STATUS_BAD_CHARGER;
		init_led_animation_mid_power_error();
	}
		break;
	case USB_STATUS_BAD_CHARGER_LOW_POWER_ATTACHED:
	{
		CurrentDeviceStatus = USB_STATUS_BAD_CHARGER;
		init_led_animation_low_power_error();
	}
		break;
	case USB_STATUS_JUST_CONNECTED:
	{
		CurrentDeviceStatus = USB_STATUS_CONNECTED;
		init_usb_connected_led_animation();
	}
		break;
	case USB_STATUS_JUST_DISCONNECTED:
	{
		CurrentDeviceStatus = DEVICE_STATUS_IDLE;
		if (MagicFlag == DFU_FLAG_VALUE)
		{
			EnterDFUMode_Reset();
		}
		init_main_led_animation();
	}
		break;
	case BATTERY_STATUS_CHARGING_STARTED:
	{
		CurrentDeviceStatus = BATTERY_STATUS_CHARGING;
		init_charging_led_animation();
	}
		break;
	case BATTERY_STATUS_CHARGING_TERMINATED:
	{
		CurrentDeviceStatus = DEVICE_STATUS_IDLE;
		init_charging_termination_led_animation();
	}
		break;
	}
	led_animation_main();
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
	CurrentDeviceStatus = USB_STATUS_JUST_DISCONNECTED;
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
	CurrentDeviceStatus = USB_STATUS_JUST_CONNECTED;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(nAlert_Pin == GPIO_Pin) {
		bool received = false;
		if(HAL_OK == STUSB4500_CheckSourceCapsReceived(&received)) {
			if(received) {
				CheckCurrentUSBPD();
			}
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	HAL_NVIC_SystemReset();
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
