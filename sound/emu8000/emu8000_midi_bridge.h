/*
 * emu8000_midi_bridge.h
 *
 *  Created on: 02-08-2013
 *      Author: Polprzewodnikowy
 */

#ifndef EMU8000_MIDI_BRIDGE_H_
#define EMU8000_MIDI_BRIDGE_H_

#include "sound/emu8000/emu8000.h"
#include "sound/emu8000/soundfont.h"

typedef struct __attribute__ ((packed)) {
	uint8_t cc[128];
	uint8_t program;
	uint16_t pitch;
	int presetnr;
} midi_ch_t;

typedef struct __attribute__ ((packed)) {
	uint16_t set[End_Oper];
	_Bool ovr[End_Oper];
} enumerators_t;

void reg_soundfont(SFInfo *sf);
void bridge_init(void);
void all_notes_off(uint8_t ch);
void reset_all_controllers(uint8_t ch);
void note_off(uint8_t ch, uint8_t note, uint8_t vel);
void note_on(uint8_t ch, uint8_t note, uint8_t vel);
void aftertouch_poly(uint8_t ch, uint8_t note, uint8_t pressure);
void control_change(uint8_t ch, uint8_t cc, uint8_t val);
void program_change(uint8_t ch, uint8_t program);
void aftertouch_chan(uint8_t ch, uint8_t pressure);
void pitch_bend(uint8_t ch, uint8_t lsb, uint8_t msb);
void sysex(uint8_t *buf, int len, uint8_t divided);

#endif /* EMU8000_MIDI_BRIDGE_H_ */
