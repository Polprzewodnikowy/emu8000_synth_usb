/*
 * led.c
 *
 *  Created on: 22-07-2013
 *      Author: Polprzewodnikowy
 */

#include "misc/led.h"

struct {
	int time;
	volatile unsigned long *ptr;
} led[LEDS];

void led_reg_ptr(int i, volatile unsigned long *ptr)
{
	if(i < LEDS)
		led[i].ptr = ptr;
}

void led_blink(int i, int time)
{
	if(i < LEDS)
		led[i].time = time;
}

void led_process(void)
{
	for(int i = 0; i < LEDS; i++)
	{
		if(led[i].time)
		{
			--led[i].time;
			if(led[i].ptr != 0)
				*led[i].ptr = 1;
		}else{
			if(led[i].ptr != 0)
				*led[i].ptr = 0;
		}
	}
}
