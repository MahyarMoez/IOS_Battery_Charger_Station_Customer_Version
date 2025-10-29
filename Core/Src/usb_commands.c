/*
 * dfa.c
 *
 *  Created on: Jul 5, 2024
 *      Author: Mahyar Ahmadi Moez
 */

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <usb_commands.h>


const uint8_t DFA_COMMAND_HEADER_SIZE = 7;

static uint32_t byteArrayToUint32LE(const uint8_t* byteArray)
{
    uint32_t result = ((uint32_t)(byteArray[0]) << 24) |
                      ((uint32_t)(byteArray[1]) << 16) |
                      ((uint32_t)(byteArray[2]) << 8) |
                      (uint32_t)(byteArray[3]);
    return result;
}

void set_command(UsbCommandData *usb_coomands, uint8_t *Buf, uint32_t Len) {
	usb_coomands->status = USB_COMMAND_STATUS_ERROR;

	if(Len<DFA_COMMAND_HEADER_SIZE) {
		usb_coomands->command = USB_COMMAND_NOP;
		return;
	}

	usb_coomands->command = Buf[0];
	usb_coomands->address = byteArrayToUint32LE(&Buf[1]);
	usb_coomands->memSize = (Buf[5] << 8) | Buf[6];
	usb_coomands->dataLength = Len - DFA_COMMAND_HEADER_SIZE;

	switch (usb_coomands->command)
	{
	case USB_COMMAND_GET_APP_VERSION:
	case USB_COMMAND_EN_POWER:
	case USB_COMMAND_USB_RESET:
	case USB_COMMAND_USB_BOOT:
	case USB_COMMAND_BOOT_PINS_INPUT:
	case USB_COMMAND_BOOT_PINS_OUTPUT:
	case USB_COMMAND_ENTER_DFU_MODE:
		usb_coomands->status = USB_COMMAND_STATUS_OK;
		break;
	default:
	{
		usb_coomands->command = USB_COMMAND_NOP;
		return;
	}
	break;
	}
}
