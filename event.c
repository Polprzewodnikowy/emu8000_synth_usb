/*
 * event.c
 *
 *  Created on: 17-08-2013
 *      Author: Polprzewodnikowy
 */

#include "stm32f4xx.h"
#include "event.h"

event_t event[EVENT_BUF];
event_t *ievent = event, *oevent = event;
volatile uint8_t cntevent = 0;

uint8_t event_any(void)
{
	if(cntevent == 0)
		return 0;
	else
		return 1;
}

void event_put(event_t ev)
{
	__disable_irq();
	*ievent++ = ev;
	cntevent++;
	if(ievent >= event + EVENT_BUF)
		ievent = event;
	__enable_irq();
}

event_t *event_get(void)
{
	__disable_irq();
	event_t *ev;
	ev = oevent++;
	if(oevent >= event + EVENT_BUF)
		oevent = event;
	cntevent--;
	__enable_irq();
	return ev;
}
