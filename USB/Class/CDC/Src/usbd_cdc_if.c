/**
 ******************************************************************************
 * @file    usbd_cdc_if_template.c
 * @author  MCD Application Team
 * @brief   Generic media access Layer.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "usb_commands.h"
#include "main.h"
#include "app_version.h"

#include <stdio.h>
#include <stdbool.h>

UsbCommandData usb_command = {USB_COMMAND_STATUS_NOT_INIT, USB_COMMAND_NOP, 0, 0, 0};
extern volatile uint32_t MagicFlag;
extern const uint32_t DFU_FLAG_VALUE;

/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */


/** @defgroup USBD_CDC
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_CDC_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */


/** @defgroup USBD_CDC_Private_Defines
 * @{
 */
/**
 * @}
 */


/** @defgroup USBD_CDC_Private_Macros
 * @{
 */

/**
 * @}
 */

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
USBD_HandleTypeDef hUsbDeviceFS;

/** @defgroup USBD_CDC_Private_FunctionPrototypes
 * @{
 */
static int8_t TEMPLATE_Init(void);
static int8_t TEMPLATE_DeInit(void);
static int8_t TEMPLATE_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t TEMPLATE_Receive(uint8_t *pbuf, uint32_t *Len);
static int8_t TEMPLATE_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);
static uint8_t TEMPLATE_Transmit(uint8_t* Buf, uint16_t Len);

static void ConfigureBootPinsOutput();
static void ConfigureBootPinsInput();

const static uint8_t AckString[2] = {'O', 'K'};
static void sendOK()
{
	TEMPLATE_Transmit((uint8_t*)AckString, sizeof(AckString)/sizeof(AckString[0]));
}

static void ConfigureBootPinsOutput()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	HAL_GPIO_WritePin(GPIOB, USB3_BOOT_Pin|USB1_BOOT_Pin|USB2_BOOT_Pin, GPIO_PIN_RESET);
	/*Configure GPIO pins : USB3_BOOT_Pin USB1_BOOT_Pin USB2_BOOT_Pin USB_RESET_Pin */
	GPIO_InitStruct.Pin = USB3_BOOT_Pin|USB1_BOOT_Pin|USB2_BOOT_Pin|USB_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void ConfigureBootPinsInput()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : USB3_BOOT_Pin USB1_BOOT_Pin USB2_BOOT_Pin USB_RESET_Pin */
	GPIO_InitStruct.Pin = USB3_BOOT_Pin|USB1_BOOT_Pin|USB2_BOOT_Pin|USB_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

USBD_CDC_ItfTypeDef USBD_CDC_Template_fops =
{
		TEMPLATE_Init,
		TEMPLATE_DeInit,
		TEMPLATE_Control,
		TEMPLATE_Receive,
		TEMPLATE_TransmitCplt
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  TEMPLATE_Init
 *         Initializes the CDC media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_Init(void)
{
	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
	return (0);
}

/**
 * @brief  TEMPLATE_DeInit
 *         DeInitializes the CDC media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_DeInit(void)
{
	/*
     Add your deinitialization code here
	 */
	return (0);
}


/**
 * @brief  TEMPLATE_Control
 *         Manage the CDC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
	UNUSED(length);

	switch (cmd)
	{
	case CDC_SEND_ENCAPSULATED_COMMAND:
		/* Add your code here */
		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:
		/* Add your code here */
		break;

	case CDC_SET_COMM_FEATURE:
		/* Add your code here */
		break;

	case CDC_GET_COMM_FEATURE:
		/* Add your code here */
		break;

	case CDC_CLEAR_COMM_FEATURE:
		/* Add your code here */
		break;

	case CDC_SET_LINE_CODING:
		/* Add your code here */
		break;

	case CDC_GET_LINE_CODING:
		/* Add your code here */
		break;

	case CDC_SET_CONTROL_LINE_STATE:
		/* Add your code here */
		break;

	case CDC_SEND_BREAK:
		/* Add your code here */
		break;

	default:
		break;
	}

	return (0);
}

/**
 * @brief  TEMPLATE_Receive
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will issue a NAK packet on any OUT packet received on
 *         USB endpoint until exiting this function. If you exit this function
 *         before transfer is complete on CDC interface (ie. using DMA controller)
 *         it will result in receiving more data while previous ones are still
 *         not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_Receive(uint8_t *Buf, uint32_t *Len)
{
	set_command(&usb_command, Buf, *Len);

	if(usb_command.status != USB_COMMAND_STATUS_OK) {
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
		USBD_CDC_ReceivePacket(&hUsbDeviceFS);
		return (USBD_OK);
	}


	switch(usb_command.command) {
	case USB_COMMAND_GET_APP_VERSION:
		TEMPLATE_Transmit((uint8_t*)APP_VERSION, strlen(APP_VERSION));
	break;
	case USB_COMMAND_BOOT_PINS_OUTPUT:
	{
		ConfigureBootPinsOutput();
		sendOK();
	}
		break;
	case USB_COMMAND_BOOT_PINS_INPUT:
	{
		ConfigureBootPinsInput();
		sendOK();
	}
		break;
	case USB_COMMAND_USB_RESET:
	{
		HAL_GPIO_WritePin(GPIOB, USB_RESET_Pin, usb_command.address == 0 ?  GPIO_PIN_SET : GPIO_PIN_RESET);
		sendOK();
	}
	break;
	case USB_COMMAND_USB_BOOT:
	{
		uint8_t port = (usb_command.address & 0xF0) >> 4;

		if(port>1 && port<=4) {
			uint8_t enable = usb_command.address & 0x0F;
			HAL_GPIO_WritePin(GPIOB, port == 2 ? USB1_BOOT_Pin : (port == 3 ? USB2_BOOT_Pin : USB3_BOOT_Pin),
					enable ?  GPIO_PIN_SET : GPIO_PIN_RESET);
			sendOK();
		}
		else {
			usb_command.status = USB_COMMAND_STATUS_ERROR;
		}
	}
	break;
	case USB_COMMAND_EN_POWER:
	{
		uint8_t port = (usb_command.address & 0xF0) >> 4;

		if(port>1 && port<=4) {
			uint8_t enable = usb_command.address & 0x0F;
			HAL_GPIO_WritePin(GPIOA, port == 2 ? Power_EN1_Pin : (port == 3 ? Power_EN2_Pin : Power_EN3_Pin),
					enable ?  GPIO_PIN_RESET : GPIO_PIN_SET);
			sendOK();
		}
		else {
			usb_command.status = USB_COMMAND_STATUS_ERROR;
		}
	}
	break;
	case USB_COMMAND_ENTER_DFU_MODE:
	{
		if(MagicFlag == 0) {
			MagicFlag = DFU_FLAG_VALUE;
			sendOK();
		}
	}
		break;
	case USB_COMMAND_NOP:
	default:
	{
		usb_command.status = USB_COMMAND_STATUS_ERROR;
	}
	break;
	};

	USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);

	if(usb_command.status != USB_COMMAND_STATUS_OK) {
		Error_Handler();
	}

	return (USBD_OK);
}

/**
 * @brief  TEMPLATE_TransmitCplt
 *         Data transmitted callback
 *
 *         @note
 *         This function is IN transfer complete callback used to inform user that
 *         the submitted Data is successfully sent over USB.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
	UNUSED(Buf);
	UNUSED(Len);
	UNUSED(epnum);

	return (0);
}


uint8_t TEMPLATE_Transmit(uint8_t* Buf, uint16_t Len)
{
	uint8_t result = USBD_OK;
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0){
		return USBD_BUSY;
	}
	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
	result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
	return result;
}

#ifdef INC_EEPROM_H_

#endif

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

