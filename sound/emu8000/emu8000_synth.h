/*
 * emu8000_synth.h / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#ifndef EMU8000_SYNTH_H_
#define EMU8000_SYNTH_H_

#include <inttypes.h>
#include "sound/emu8000/emu8000.h"

#define SET_VOLUME		(1<<0)
#define SET_PITCH		(1<<1)
#define SET_PAN			(1<<2)
#define SET_FMMOD		(1<<3)
#define SET_TREMFRQ		(1<<4)
#define SET_FM2FRQ2		(1<<5)
#define SET_Q			(1<<6)

#define STATE_END		0
#define STATE_PLAYING	(1<<1)
#define STATE_RELEASED	(1<<2)
#define STATE_OFF		(1<<3)

typedef struct {
	uint32_t start_address;
	uint32_t loop_start;
	uint32_t loop_end;
	uint8_t attenuation;
	uint16_t initial_pitch;
	uint8_t initial_cutoff;
	uint16_t pitch_target;
	uint16_t volume_target;
	uint16_t cutoff_target;
	uint8_t pitch_height;
	uint8_t cutoff_height;
	uint8_t pan;
	uint8_t reverb;
	uint8_t chorus;
	uint8_t aux;
	uint8_t filter_q;
	uint16_t volenv_delay;
	uint8_t volenv_attack;
	uint8_t volenv_hold;
	uint8_t volenv_decay;
	uint8_t volenv_sustain;
	uint8_t volenv_release;
	uint16_t modenv_delay;
	uint8_t modenv_attack;
	uint8_t modenv_hold;
	uint8_t modenv_decay;
	uint8_t modenv_sustain;
	uint8_t modenv_release;
	uint16_t lfo1_delay;
	uint8_t lfo1_frequency;
	uint8_t lfo1_vibrato;
	uint8_t lfo1_cutoff;
	uint8_t lfo1_tremolo;
	uint16_t lfo2_delay;
	uint8_t lfo2_frequency;
	uint8_t lfo2_vibrato;
} emu8000_reg_t;

typedef struct {
	emu8000_reg_t reg;
	uint8_t state;
	uint8_t ch;
	uint8_t note;
	uint8_t vel;
	uint8_t exclusive_class;
	uint8_t loop;
	uint8_t vol;
} emu8000_voice_t;

void reset_regs(emu8000_reg_t *reg);
void emu8000_start_voice(uint8_t ch, emu8000_reg_t reg);
void emu8000_update_voice(uint8_t ch, emu8000_reg_t reg, int update);
void emu8000_release_voice(uint8_t ch, emu8000_reg_t reg);
void emu8000_terminate_voice(uint8_t ch);
void emu8000_reset_voice(uint8_t ch);
uint8_t emu8000_find_voice(emu8000_voice_t *voice);

#endif /* EMU8000_SYNTH_H_ */
