/*
 * usbd_audio.h
 *
 *  Created on: 31-03-2013
 *      Author: Polprzewodnikowy
 */

#ifndef USBD_AUDIO_H_
#define USBD_AUDIO_H_

#include "usb_core.h"

extern USBD_Class_cb_TypeDef MIDI_cb;
extern const uint8_t MIDI_CIN_Size[];

#define MIDI_NOTE_OFF			0x80
#define MIDI_NOTE_ON			0x90
#define MIDI_AFTERTOUCH_POLY	0xA0
#define MIDI_CONTROL_CHANGE		0xB0
#define MIDI_PROGRAM_CHANGE		0xC0
#define MIDI_AFTERTOUCH_CHAN	0xD0
#define MIDI_PITCH_BEND			0xE0
#define MIDI_SYSEX				0xF0
#define MIDI_SYSEX_END			0xF7

#endif /* USBD_AUDIO_H_ */
