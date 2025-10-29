/*
 * ws2812.cpp
 *
 *  Created on: Jul 5, 2024
 *      Author: Mahyar Ahmadi Moez
 */

#include "ws2812.h"

#include "stm32g4xx_hal.h"

extern TIM_HandleTypeDef htim3;

#define LED_DATA_SIZE  (COMMAND_MAX_LED*24)

#define WS2812_RESET_LENGTH 50

#define WS2812_BIT_HIGH  86
#define WS2812_BIT_LOW 	 44

static uint8_t LEDBuffer[LED_DATA_SIZE + WS2812_RESET_LENGTH];

void ws2812_send (command_led_color_t * led_data){

	uint32_t color;

	for (int i= 0; i<COMMAND_MAX_LED; i++){
		color = ((led_data[i].green<<16) | (led_data[i].red<<8) | (led_data[i].blue));

		for (int j=0; j<24; j++){
			if (color&(1<<(23-j))){
				LEDBuffer[(i*24)+j] = WS2812_BIT_HIGH;
			}
			else {
				LEDBuffer[(i*24)+j] = WS2812_BIT_LOW;
			}
		}
	}

	HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_2, (uint32_t *)&LEDBuffer, LED_DATA_SIZE + WS2812_RESET_LENGTH);
}
