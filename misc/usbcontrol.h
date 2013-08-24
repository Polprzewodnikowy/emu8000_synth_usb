/*
 * usbcontrol.h
 *
 *  Created on: 21-08-2013
 *      Author: Polprzewodnikowy
 */

#ifndef USBCONTROL_H_
#define USBCONTROL_H_

#include "sound/sb/sb_mixer.h"
#include "sound/emu8000/emu8000.h"

#define USBCONTROL_RESPONSE				0xDD

#define USBCONTROL_CONNECTED			1
#define USBCONTROL_DISCONNECTED			2
#define USBCONTROL_SET_MIXER_FEATURE	3
#define USBCONTROL_SET_EMU8000_FEATURE	4
#define USBCONTROL_GET_SETTINGS			5

#define USBCONTROL_MIXER_L				1
#define USBCONTROL_MIXER_R				2

#define USBCONTROL_MIXER_MASTER			1
#define USBCONTROL_MIXER_VOICE			2
#define USBCONTROL_MIXER_MIDI			3
#define USBCONTROL_MIXER_BASS			4
#define USBCONTROL_MIXER_TREBLE			5
#define USBCONTROL_MIXER_3DSE			6

#define USBCONTROL_EMU8000_EQ			1
#define USBCONTROL_EMU8000_REVERB		2
#define USBCONTROL_EMU8000_CHORUS		3

void usbcontrol_process_data(uint8_t *data);

#endif /* USBCONTROL_H_ */
