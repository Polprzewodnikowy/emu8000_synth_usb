/*
 * event.h
 *
 *  Created on: 17-08-2013
 *      Author: Polprzewodnikowy
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <inttypes.h>

#define EVENT_BUF	64

typedef enum {
	EVENT_TYPE_NULL = 0,
	EVENT_TYPE_MIDI,
	EVENT_TYPE_USB,
} event_type_t;

typedef struct {
	event_type_t type;
	uint8_t buf[64];
} event_t;

uint8_t event_any(void);
void event_put(event_t ev);
event_t *event_get(void);

#endif /* EVENT_H_ */
