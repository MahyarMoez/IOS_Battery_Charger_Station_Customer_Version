/*
 * led_animation.h
 *
 *  Created on: Jun 11, 2021
 *      Author: Tobias
 */

#ifndef SRC_MODULES_LED_ANIMATION_H_
#define SRC_MODULES_LED_ANIMATION_H_

#include "ws2812.h"

/**
 * Initialize the module
 */
void init_main_led_animation(void);
void init_led_animation_low_power_error();
void init_led_animation_mid_power_error();
void init_led_animation_power_error();
void init_usb_connected_led_animation(void);
void init_charging_led_animation(void);
void init_charging_termination_led_animation(void);

/**
 * Set all LEDs Black and DeInit the module
 */
void led_animation_deinit(void);

/**
 * Go to a specific frame
 */
void led_animation_go_to_frame(uint32_t id);

/**
 * Main loop for the animation core
 */
void led_animation_main(void);

#endif /* SRC_MODULES_LED_ANIMATION_H_ */
