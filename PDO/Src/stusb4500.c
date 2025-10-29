/*
 * usbpd_nvm.h
 *
 *  Created on: Oct 6, 2025
 *      Author: Mahyar Moez
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stusb4500.h"

#define STUSB4500_I2C_ADDR     			(0x28 << 1)  // HAL expects 8-bit address
#define REG_ALERT_STATUS_1				0x0B
#define REG_ALERT_STATUS_1_MASK 		0x0C
#define REG_PORT_STATUS_0       		0x0D
//#define REG_PORT_STATUS_1       		0x0E
#define REG_TYPEC_MONITORING_STATUS_0 	0x0F
#define REG_CC_STATUS					0x11
#define REG_CC_HW_FAULT_STATUS_0		0x12
#define REG_PD_TYPEC_STATUS				0x14
#define REG_PRT_STATUS					0x16
#define REG_PD_COMMAND_CTRL     		0x1A
#define REG_RESET_CTRL         			0x23
#define REG_PE_FSM						0x29
//#define REG_DEVICE_ID         			0x2F
#define REG_RX_HEADER_LOW				0x31
#define REG_RX_DATA_OBJ1_0				0x33
#define REG_TX_HEADER					0x51
#define REG_DPM_PDO_NUMB       			0x70
#define REG_PDO1               			0x85
//#define REG_PDO2                		0x89
//#define REG_PDO3               		0x8D
#define REG_RDO_REG_STATUS_0    		0x91

// NVM registers
#define REG_NVM_COMMAND        			0x95
#define REG_NVM_STATUS         			0x96

#define SEND_COMMAND 					0x26
#define SW_RESET_OFF	        		0x00
#define SW_RESET_ON           			0x01
#define NVM_BUSY_MASK          			0x01
#define NVM_PROG_EN_CMD        			0x47

#define SNK_PDO_NUMB 3  // number of PDOs used (1-3)

typedef struct
{
	uint16_t  voltage;   // in mV
	uint16_t  current;   // in mA
} PDO_t;

typedef union
{
  uint32_t d32;
  struct
  {
        uint32_t MaxCurrent                     :       10;
        uint32_t OperatingCurrent               :       10;
        uint8_t reserved_22_20                  :       3;
        uint8_t UnchunkedMess_sup               :       1;
        uint8_t UsbSuspend                      :       1;
        uint8_t UsbComCap                       :       1;
        uint8_t CapaMismatch                    :       1;
        uint8_t GiveBack                        :       1;
        uint8_t Object_Pos                      :       3;
        uint8_t reserved_31		        		:		1;

  } b;
} STUSB_RDO_REG_STATUS_RegTypeDef;

PDO_t PDOs_Defaults[SNK_PDO_NUMB] = {{5000, 500}, {15000, 700}, {20000, 600}};  // 5V is standard compulsion

void ITM_Print(const char *str) {
    if (!str) return;
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0) return; // ITM not enabled
    if ((ITM->TER & (1UL << 0)) == 0) return;         // Port 0 not enabled

    while (*str) {
        ITM_SendChar(*str++);
    }
}

void ITM_PrintInt(int32_t value) {
    char str[12];
    snprintf(str, sizeof(str), "%ld", value);

    ITM_Print(str);
}

void ITM_PrintHex(unsigned int value) {
    char str[12];
    snprintf(str, sizeof(str), "0X%X", value);

    ITM_Print(str);
}

static bool PDO_equal(PDO_t a, PDO_t b) {
    return (a.voltage == b.voltage) && (a.current == b.current);
}

static uint32_t STUSB4500_EncodePDO(const PDO_t pdo)
{
    uint32_t voltage = (uint32_t)(pdo.voltage / 50);  // 50 mV units
    uint32_t current = (uint32_t)(pdo.current / 10);  // 10 mA units
    return (voltage << 10) | (current);
}

static HAL_StatusTypeDef STUSB4500_WriteReg(uint8_t reg, uint8_t *value, uint16_t length)
{
    return HAL_I2C_Mem_Write(&hi2c2, STUSB4500_I2C_ADDR, reg,
                             I2C_MEMADD_SIZE_8BIT, value, length, 20);
}

static HAL_StatusTypeDef STUSB4500_ReadReg(uint8_t reg, uint8_t *value, uint16_t length)
{
    return HAL_I2C_Mem_Read(&hi2c2, STUSB4500_I2C_ADDR, reg,
                            I2C_MEMADD_SIZE_8BIT, value, length, 20);
}

static HAL_StatusTypeDef STUSB4500_WriteDefaultPDOs(void)
{
    uint8_t buffer[(SNK_PDO_NUMB) * 4];  // 3 PDOs × 4 bytes
    for(int index=0; index<SNK_PDO_NUMB; ++index) {
    	uint32_t pdo = STUSB4500_EncodePDO(PDOs_Defaults[index]);
    	memcpy(&buffer[index*4],  &pdo, 4);
    }

    HAL_StatusTypeDef status = STUSB4500_WriteReg(REG_PDO1, buffer, sizeof(buffer)/sizeof(buffer[0]));
    if (status != HAL_OK)
        return status;

    uint8_t total_pdos = SNK_PDO_NUMB;
    return STUSB4500_WriteReg(REG_DPM_PDO_NUMB, &total_pdos, sizeof(total_pdos));
}

static HAL_StatusTypeDef STUSB4500_ProgramNVM(void)
{
    // Trigger NVM programming
	uint8_t value = NVM_PROG_EN_CMD;
	HAL_StatusTypeDef status = STUSB4500_WriteReg(REG_NVM_COMMAND, &value, 1);
    if(HAL_OK != status) {
    	return status;
    }

    HAL_Delay(50);

    // Poll until NVM_BUSY bit clears
    uint8_t reg = 0;
    do {
        status = STUSB4500_ReadReg(REG_NVM_STATUS, &reg, 1);
        if(HAL_OK != status) {
        	return status;
        }
    } while (reg & NVM_BUSY_MASK);

    return status;
}

static HAL_StatusTypeDef STUSB4500_SoftwareReset(void)
{
    uint16_t Data16 = 0x000D;
    uint8_t buff[2];
    buff[0] = Data16 & 0xFF;
    buff[1] = (Data16 >> 8) & 0xFF;

	HAL_StatusTypeDef status = STUSB4500_WriteReg(REG_TX_HEADER, buff, sizeof(buff)/sizeof(buff[0]));
	if(HAL_OK != status) {
		return status;
	}

	uint8_t reg = SEND_COMMAND;
	status = STUSB4500_WriteReg(REG_PD_COMMAND_CTRL, &reg, sizeof(reg));
	if(HAL_OK != status) {
		return status;
	}

	HAL_Delay(30);

	return status;
}

static HAL_StatusTypeDef STUSB4500_ClearStatusRegisters(void)
{
	uint8_t reg = 0;
    HAL_StatusTypeDef status = STUSB4500_ReadReg(REG_ALERT_STATUS_1, &reg, sizeof(reg));
    if(HAL_OK != status) {
    	return status;
    }

    status = STUSB4500_ReadReg(REG_PORT_STATUS_0, &reg, sizeof(reg));
    if(HAL_OK != status) {
    	return status;
    }

    status = STUSB4500_ReadReg(REG_TYPEC_MONITORING_STATUS_0, &reg, sizeof(reg));
    if(HAL_OK != status) {
    	return status;
    }

    status = STUSB4500_ReadReg(REG_CC_HW_FAULT_STATUS_0, &reg, sizeof(reg));
    if(HAL_OK != status) {
    	return status;
    }

    status = STUSB4500_ReadReg(REG_PD_TYPEC_STATUS, &reg, sizeof(reg));
    if(HAL_OK != status) {
    	return status;
    }

    return STUSB4500_ReadReg(REG_PRT_STATUS, &reg, sizeof(reg));
}

HAL_StatusTypeDef STUSB4500_Initialize(void)
{
	HAL_StatusTypeDef status = STUSB4500_WriteDefaultPDOs();   // Set up PDO2–PDO3 and PDO count
    if(HAL_OK != status) {
    	return status;
    }

    status = STUSB4500_ProgramNVM();         // Commit to NVM
    if(HAL_OK != status) {
    	return status;
    }

    return STUSB4500_SoftwareReset();      // Reload from NVM
}

HAL_StatusTypeDef STUSB4500_ReadBackPDOs(bool *is_ok)
{
	*is_ok = false;

    uint8_t buf[12];
    HAL_StatusTypeDef status = STUSB4500_ReadReg(REG_PDO1, buf, sizeof(buf)/sizeof(buf[0]));
	if(HAL_OK != status) {
		return status;
	}

	status = STUSB4500_ClearStatusRegisters();
	if(HAL_OK != status) {
		return status;
	}

	uint8_t reg = 0xFF;
	// enable alerts with un-masking them: PRT_STATUS_AL, CC_HW_FAULT_STATUS_AL, TYPEC_MONITORING_STATUS_AL, PORT_STATUS_AL
    reg &= ~((1<<6) | (1<<5) | (1<<4) | (1<<1));
    status = STUSB4500_WriteReg(REG_ALERT_STATUS_1_MASK ,&reg, sizeof(reg));
	if(HAL_OK != status) {
		return status;
	}

	uint32_t pdo = 0;
	PDO_t PDOInfo[SNK_PDO_NUMB] = {0};

	for(uint8_t i=0; i<SNK_PDO_NUMB; ++i) {
		memcpy(&pdo, &buf[4*i], 4);
		PDOInfo[i].voltage = ((pdo >> 10) & 0x3FF) * 50;
		PDOInfo[i].current = (pdo & 0x3FF) * 10;

		ITM_Print("PDO ");
		ITM_PrintInt(i+1);
		ITM_Print(" voltage: ");
		ITM_PrintInt(PDOInfo[i].voltage);
		ITM_Print(" current: ");
		ITM_PrintInt(PDOInfo[i].current);
		ITM_Print("\n");
	}

	if(PDO_equal(PDOInfo[0], PDOs_Defaults[0]) && PDO_equal(PDOInfo[1], PDOs_Defaults[1]) && PDO_equal(PDOInfo[2], PDOs_Defaults[2])) {
		*is_ok = true;
	}

	return status;
}

HAL_StatusTypeDef STUSB4500_CheckSourceCapsReceived(bool *received)
{
	*received = false;
	uint8_t reg = 0;
    HAL_StatusTypeDef status = STUSB4500_ReadReg(REG_ALERT_STATUS_1, &reg, sizeof(reg));
    if(HAL_OK != status) {
    	return status;
    }

    // port alert
    if((reg & 0x02) == 0x02) {
        status = STUSB4500_ReadReg(REG_PRT_STATUS, &reg, sizeof(reg));
        if(HAL_OK != status) {
        	return status;
        }

        if((reg & 0x04) == 0x04) // PRL_MSG_RECEIVED
        {
        	*received = true;
        }
    }

    return STUSB4500_ClearStatusRegisters();
}

HAL_StatusTypeDef STUSB4500_ReadCurrentPDO(uint8_t *total_charging_ports)
{
	*total_charging_ports = 0;
	HAL_StatusTypeDef status = HAL_ERROR;

	uint8_t buff[7 * 4];

	status = STUSB4500_ReadReg(REG_RX_HEADER_LOW, buff, 2); // read all 2-bytes header
	if(HAL_OK != status) {
		return status;
	}

	uint8_t ps_rdy  = (buff[0] & 0x07) == 0x06;
	if (ps_rdy == 0) {
		ITM_Print("power supply is not ready!\n");
		uint8_t reg = 0;

		status = STUSB4500_ReadReg(REG_PE_FSM, &reg, sizeof(reg));
		if(HAL_OK == status) {
			if((reg >> 4) == 0x03) { // hard reset!
				HAL_GPIO_WritePin(GPIOC, LED_G_Pin, GPIO_PIN_SET);
				return HAL_BUSY;
			}
		}

		status = STUSB4500_ReadReg(REG_CC_STATUS, &reg, sizeof(reg));
		if(HAL_OK == status) {
			uint8_t cc1_state = reg & 0x03;
			uint8_t cc2_state = (reg >> 2) & 0x03;

			if((cc1_state == 0x03 || cc2_state == 0x03)) {
				return HAL_BUSY;
			}
		}
		return status;
	}

	status = STUSB4500_ReadReg(REG_RX_DATA_OBJ1_0, buff, sizeof(buff)/sizeof(buff[0]));
	if(HAL_OK != status) {
		return status;
	}

	STUSB_RDO_REG_STATUS_RegTypeDef rdo_reg;
	status = STUSB4500_ReadReg(REG_RDO_REG_STATUS_0, (uint8_t *)&rdo_reg.d32, sizeof(rdo_reg.d32));
    if(HAL_OK != status) {
    	return status;
    }

    uint8_t object_pos = rdo_reg.b.Object_Pos;
	// Negotiation failed or has not happened
	if(object_pos == 0 || object_pos>7) {
		ITM_Print("object_pos is wrong!\n");
		return status;
	}

	uint32_t pd_src;
	memcpy(&pd_src, &buff[(object_pos - 1) * 4], sizeof(pd_src));

	uint8_t type = (pd_src >> 30) & 0x03;
	uint16_t voltage_mV = ((pd_src >> 10) & 0x3FF) * 50;
	uint16_t max_current_mA = rdo_reg.b.MaxCurrent * 10;

	ITM_Print("---------------------------\n");
	ITM_Print("Current PDO: ");
	ITM_PrintHex(pd_src);
	ITM_Print(" position: ");
	ITM_PrintInt(object_pos);
	ITM_Print(" OperatingCurrent: ");
	ITM_PrintInt(rdo_reg.b.OperatingCurrent * 10);
	ITM_Print(" Max current: ");
	ITM_PrintInt(max_current_mA);
	ITM_Print(" voltage: ");
	ITM_PrintInt(voltage_mV);
	ITM_Print(" type: ");
	ITM_Print((type == 0) ? "Fixed" : (type == 1) ? "Battery" : ((type == 2) ? "Variable" : "Reserved"));
	ITM_Print("\n");


	// Discarding none-fixed supplies
	if(type != 0) {
		ITM_Print("none-fixed supplies!\n");
		return status;
	}

	// just consider it as bad charger
	if(object_pos == 1 && abs(voltage_mV - 5000) > 500) {
		return status;
	}

	uint32_t total_capabilities = (uint32_t)voltage_mV * max_current_mA / 1000 / 12000; // 12 W : minimum power supply for charging one battery
	if(total_capabilities > 3) {
		total_capabilities = 3;
	}
	*total_charging_ports = (uint8_t)total_capabilities;


	return status;
}
