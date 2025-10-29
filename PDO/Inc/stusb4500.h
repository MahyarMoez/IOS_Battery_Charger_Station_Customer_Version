/*
 * usbpd_nvm.h
 *
 *  Created on: Oct 6, 2025
 *      Author: Mahyar Moez
 */

#ifndef SRC_STUSB4500_NVM_H_
#define SRC_STUSB4500_NVM_H_

#include <stdbool.h>
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

HAL_StatusTypeDef STUSB4500_Initialize(void);
HAL_StatusTypeDef STUSB4500_ReadBackPDOs(bool *is_ok);
HAL_StatusTypeDef STUSB4500_ReadCurrentPDO(uint8_t *total_charging_ports);
HAL_StatusTypeDef STUSB4500_CheckSourceCapsReceived(bool *received);

#endif /* SRC_STUSB4500_NVM_H_ */
