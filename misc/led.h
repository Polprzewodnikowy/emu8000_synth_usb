/*
 * led.h
 *
 *  Created on: 22-07-2013
 *      Author: Polprzewodnikowy
 */

#ifndef LED_H_
#define LED_H_

#define LEDS	4

#define LED_G	0
#define LED_O	1
#define LED_R	2
#define LED_B	3

void led_reg_ptr(int i, volatile unsigned long *ptr);
void led_blink(int i, int time);
void led_process(void);

#endif /* LED_H_ */
