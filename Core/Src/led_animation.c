/*
 * led_animation.c
 *
 *  Created on: Jul 5, 2024
 *      Author: Mahyar Ahmadi Moez
 */
#include "led_animation.h"

#include "stm32g4xx_hal.h"

#include <string.h>
#include <stdbool.h>
#include <math.h>

static void led_animation_fade_frame(uint32_t time, uint32_t maxtime, command_frame_t *current, command_frame_t *from, command_frame_t *to);
static void led_animation_fade(uint32_t time, uint32_t maxtime, command_led_color_t *current, const command_led_color_t *from, const command_led_color_t *to);
static void init_blinking_led_animation_buffer(const command_led_color_t color);
static void init_fixed_led_animation_buffer(const command_led_color_t color);

static command_frame_t CurrentFrameStorage[COMMAND_NUMBER_OF_FRAMES] = {0};

static uint32_t TimeElapsed = 0;
static command_frame_t CurrentFrame;

static const uint8_t TotalErrorLEDs = 3;

/**
 * Initialize the module
 */
void init_main_led_animation(void)
{
	init_fixed_led_animation_buffer(DARKBLUE);
	led_animation_go_to_frame(0);	//Jump to first frame
}

void init_led_animation_low_power_error()
{
	for(int index = 0; index< COMMAND_NUMBER_OF_FRAMES; index++)
	{
		for(int j=0;j<COMMAND_MAX_LED; j++)
		{
			CurrentFrameStorage[index].pixel_data[j] = (j < TotalErrorLEDs || j >= (COMMAND_MAX_LED - TotalErrorLEDs - 1)) ? ORANGE : DARKBLUE;
		}

		CurrentFrameStorage[index].fade = false;
		CurrentFrameStorage[index].last = 0;
		CurrentFrameStorage[index].timeout= 0;
		CurrentFrameStorage[index].id = index;
		CurrentFrameStorage[index].next = index+1;
	}

	CurrentFrameStorage[COMMAND_NUMBER_OF_FRAMES-1].next = 0;

	led_animation_go_to_frame(0);	//Jump to first frame
}

void init_led_animation_mid_power_error()
{
	for(int index = 0; index< COMMAND_NUMBER_OF_FRAMES; index++)
	{
		for(int j=0;j<COMMAND_MAX_LED; j++)
		{
			CurrentFrameStorage[index].pixel_data[j] = (j < TotalErrorLEDs) ? ORANGE : DARKBLUE;
		}

		CurrentFrameStorage[index].fade = false;
		CurrentFrameStorage[index].last = 0;
		CurrentFrameStorage[index].timeout= 0;
		CurrentFrameStorage[index].id = index;
		CurrentFrameStorage[index].next = index+1;
	}

	CurrentFrameStorage[COMMAND_NUMBER_OF_FRAMES-1].next = 0;

	led_animation_go_to_frame(0);	//Jump to first frame
}

void init_led_animation_power_error()
{
	init_fixed_led_animation_buffer(ORANGE);
	led_animation_go_to_frame(0);	//Jump to first frame
}

void init_charging_led_animation(void)
{
	init_blinking_led_animation_buffer(DARKBLUE);
	led_animation_go_to_frame(0);	//Jump to first frame
}

void init_usb_connected_led_animation(void)
{
	init_blinking_led_animation_buffer(DARKRED);
	led_animation_go_to_frame(0);	//Jump to first frame
}

void init_charging_termination_led_animation(void)
{
	init_main_led_animation();
}
/**
 * DeInit the module
 */
void led_animation_deinit(void)
{
	// reset first Frame to Black
	memset((void *)&CurrentFrame, 0x00, sizeof(command_frame_t));
	ws2812_send(CurrentFrame.pixel_data);
}

/**
 * Go to a specific frame
 */
void led_animation_go_to_frame(uint32_t id)
{
	//Switch to the next frame asap
	if (id < COMMAND_NUMBER_OF_FRAMES){
		CurrentFrame.next 		= id;
		CurrentFrame.timeout	= 0;
	}
}

/**
 * Main loop for the animation core
 */
void led_animation_main(void)
{
	static uint32_t lastTick = 0;

	uint32_t tmpLastTick = lastTick;
	lastTick = HAL_GetTick();

	if(TimeElapsed >= CurrentFrame.timeout)
	{
		uint32_t id 		= CurrentFrame.next;
		uint32_t last_id	= CurrentFrame.id;

		if (CurrentFrame.next < COMMAND_NUMBER_OF_FRAMES)
		{

			memcpy(
					&CurrentFrame,
					(command_frame_t *)&CurrentFrameStorage[id],
					sizeof(command_frame_t)
			);

			CurrentFrame.id 	= id;
			CurrentFrame.last 	= last_id;
			TimeElapsed = 0;
		}

		if(CurrentFrame.fade)
		{
			led_animation_fade_frame(TimeElapsed, CurrentFrame.timeout,
					&CurrentFrame,
					(command_frame_t *)&CurrentFrameStorage[CurrentFrame.last],
					(command_frame_t *)&CurrentFrameStorage[CurrentFrame.id]
			);
		}

	}

	//Update Leds
	ws2812_send(CurrentFrame.pixel_data);

	TimeElapsed += (lastTick - tmpLastTick);
}

void led_animation_fade(
		uint32_t time, uint32_t maxtime,
		command_led_color_t *current, const command_led_color_t *from, const command_led_color_t *to)
{
	float xfactor = (float)time/maxtime;
	current->red 	= xfactor * ((int32_t)to->red - (int32_t)from->red) + from->red;
	current->green  = xfactor * ((int32_t)to->green - (int32_t)from->green) + from->green;
	current->blue	= xfactor * ((int32_t)to->blue - (int32_t)from->blue) + from->blue;
}

void led_animation_fade_frame(
		uint32_t time, uint32_t maxtime,
		command_frame_t *current, command_frame_t *from, command_frame_t *to)
{
	for(int i = 0; i < COMMAND_MAX_LED; i++)
	{
		led_animation_fade(time, maxtime,
				&current->pixel_data[i], &from->pixel_data[i], &to->pixel_data[i]);
	}
}

void init_fixed_led_animation_buffer(const command_led_color_t color)
{
	for(int index = 0; index< COMMAND_NUMBER_OF_FRAMES; index++)
	{
		for(int j=0;j<COMMAND_MAX_LED; j++)
		{
			CurrentFrameStorage[index].pixel_data[j] = color;
		}

		CurrentFrameStorage[index].fade = false;
		CurrentFrameStorage[index].last = 0;
		CurrentFrameStorage[index].timeout= 0;
		CurrentFrameStorage[index].id = index;
		CurrentFrameStorage[index].next = index+1;
	}

	CurrentFrameStorage[COMMAND_NUMBER_OF_FRAMES-1].next = 0;
}

void init_blinking_led_animation_buffer(const command_led_color_t color)
{
	float intencity = 0;

	for(int index = 0; index< COMMAND_NUMBER_OF_FRAMES; index++)
	{
		intencity = sin(M_PI * index / (COMMAND_NUMBER_OF_FRAMES - 1));

		command_led_color_t stepColor = color;
		stepColor.blue *= intencity;
		stepColor.green *= intencity;
		stepColor.red *= intencity;

		for(int j=0; j<COMMAND_MAX_LED; j++)
		{
			CurrentFrameStorage[index].pixel_data[j] = stepColor;
		}

		CurrentFrameStorage[index].fade = true;
		CurrentFrameStorage[index].last = 0;
		CurrentFrameStorage[index].timeout= 47;
		CurrentFrameStorage[index].id = index;
		CurrentFrameStorage[index].next = index+1;
	}

	CurrentFrameStorage[0].fade = false;
	CurrentFrameStorage[0].timeout = 680;

	CurrentFrameStorage[COMMAND_NUMBER_OF_FRAMES-1].next = 0;
}
