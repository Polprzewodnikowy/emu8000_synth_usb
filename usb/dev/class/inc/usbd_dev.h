/*
 * usbd_dev.h
 *
 *  Created on: 31-03-2013
 *      Author: Polprzewodnikowy
 */

#ifndef USBD_DEV_H_
#define USBD_DEV_H_

#include "usb_core.h"

extern USBD_Class_cb_TypeDef USBD_cb;
extern const uint8_t cin_size[];

#define MIDI_NOTE_OFF			0x80
#define MIDI_NOTE_ON			0x90
#define MIDI_AFTERTOUCH_POLY	0xA0
#define MIDI_CONTROL_CHANGE		0xB0
#define MIDI_PROGRAM_CHANGE		0xC0
#define MIDI_AFTERTOUCH_CHAN	0xD0
#define MIDI_PITCH_BEND			0xE0
#define MIDI_SYSEX				0xF0
#define MIDI_SYSEX_END			0xF7

void usbd_TxPacket(uint8_t epnum, uint8_t *buf, int len);

#endif /* USBD_DEV_H_ */
