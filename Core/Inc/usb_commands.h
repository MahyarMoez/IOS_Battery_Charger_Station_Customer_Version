/*
 * dfa.h
 *
 *  Created on: Jul 5, 2024
 *      Author: Mahyar Ahmadi Moez
 */

#ifndef INC_USB_COMMANDS_H_
#define INC_USB_COMMANDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdint.h>

enum USB_COMMAND_TYPES {
	USB_COMMAND_NOP = 					 		0x00,
	USB_COMMAND_GET_APP_VERSION = 				0x05,

	USB_COMMAND_EN_POWER = 						0x70,

    USB_COMMAND_USB_RESET = 					0x81,
    USB_COMMAND_USB_BOOT = 						0x82,

    USB_COMMAND_BOOT_PINS_OUTPUT = 				0x90,
    USB_COMMAND_BOOT_PINS_INPUT = 				0x91,

    USB_COMMAND_ENTER_DFU_MODE = 				0xA0,
};

enum USB_COMMAND_STATUS {
	USB_COMMAND_STATUS_NOT_INIT = 				0x00,
	USB_COMMAND_STATUS_OK = 					0x01,

	USB_COMMAND_STATUS_ERROR = 					0xFF,
};

typedef struct UsbCommandStruct {
	uint8_t status;
	uint8_t command;
	uint16_t memSize;
	uint32_t address;
	uint16_t dataLength;
	uint8_t data[256];
} UsbCommandData;

void set_command(UsbCommandData *usb_command, uint8_t *Buf, uint32_t Len);

#ifdef __cplusplus
}
#endif

#endif /* INC_USB_COMMANDS_H_ */
