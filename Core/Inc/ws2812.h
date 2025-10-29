/*
 * ws2812.h
 *
 *  Created on: Jun 11, 2021
 *      Author: Tobias
 *
 *  Edited: Nov 4, 2022
 *  	Author: CWI
 *  	WS2812B-V5 Timing
 *  	T0H  220ns-380ns
 *  	T0L  580ns-1000us
 *
 *  	T1H  580ns-1000us
 *  	T1L  580ns-1000us
 *
 * 		Configured Timing: ~1.333us/Bit
 * 							0-155
 * 							1 = 8.333 ns
 *
 */

#include <stdint.h>

#ifndef USE_HAL_DRIVER
#pragma pack(push, 1)
#define __CMD_PACKED
#else
#define __CMD_PACKED		__attribute__((__packed__))
#endif

#ifndef SRC_MODULES_WS2812_H_
#define SRC_MODULES_WS2812_H_

#define COMMAND_MAX_LED					(40)
#define COMMAND_NUMBER_OF_FRAMES		(COMMAND_MAX_LED * 2)

typedef struct __CMD_PACKED {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} command_led_color_t;

#define BLACK		(command_led_color_t){.red = 0x00, .green = 0x00, .blue = 0x00}
#define DARKBLUE	(command_led_color_t){.red = 0x00, .green = 0x00, .blue = 0x7F}
#define DARKRED		(command_led_color_t){.red = 0x32, .green = 0x00, .blue = 0x00}
#define ORANGE		(command_led_color_t){.red = 0x7F, .green = 0x3F, .blue = 0x00}

typedef struct __CMD_PACKED {
	command_led_color_t	pixel_data[COMMAND_MAX_LED];	//24bit pixel data in BRG format
	uint16_t 			timeout;
	uint8_t 			next;
	uint8_t 			fade;
	uint8_t 			last;
	uint8_t				id; 	//Only used internally
} command_frame_t;

/**
 * send an array of colors to the leds
**/
void ws2812_send (command_led_color_t * led_data);

#endif /* SRC_MODULES_WS2812_H_ */
