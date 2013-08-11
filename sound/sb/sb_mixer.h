/*
 * sb_mixer.h / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#ifndef SB_MIXER_H_
#define SB_MIXER_H_

#include <inttypes.h>

#define SBMIX_ADDRESS			(SB_BASE_ADDRESS | 0x4)
#define SBMIX_DATA				(SB_BASE_ADDRESS | 0x5)

#define SBMIX_RESET				0x00
#define SBMIX_VOICE_LR			0x04
#define SBMIX_MIC_OLD			0x0A
#define SBMIX_MASTER_LR			0x22
#define SBMIX_MIDI_LR			0x26
#define SBMIX_CD_LR				0x28
#define SBMIX_LINE_LR			0x2E
#define SBMIX_MASTER_L			0x30
#define SBMIX_MASTER_R			0x31
#define SBMIX_VOICE_L			0x32
#define SBMIX_VOICE_R			0x33
#define SBMIX_MIDI_L			0x34
#define SBMIX_MIDI_R			0x35
#define SBMIX_CD_L				0x36
#define SBMIX_CD_R				0x37
#define SBMIX_LINE_L			0x38
#define SBMIX_LINE_R			0x39
#define SBMIX_MIC				0x3A
#define SBMIX_PCSPK				0x3B
#define SBMIX_OUTPUT_SWITCH		0x3C
#define SBMIX_INPUT_SWITCH_L	0x3D
#define SBMIX_INPUT_SWITCH_R	0x3E
#define SBMIX_INPUT_GAIN_L		0x3F
#define SBMIX_INPUT_GAIN_R		0x40
#define SBMIX_OUTPUT_GAIN_L		0x41
#define SBMIX_OUTPUT_GAIN_R		0x42
#define SBMIX_AGC				0x43
#define SBMIX_TREBLE_L			0x44
#define SBMIX_TREBLE_R			0x45
#define SBMIX_BASS_L			0x46
#define SBMIX_BASS_R			0x47
#define SBMIX_3DSE				0x90

#define SBMIX_OUTPUT_LINE_L_BIT	(1<<4)
#define SBMIX_OUTPUT_LINE_R_BIT	(1<<3)
#define SBMIX_OUTPUT_CD_L_BIT	(1<<2)
#define SBMIX_OUTPUT_CD_R_BIT	(1<<1)
#define SBMIX_OUTPUT_MIC_BIT	(1<<0)
#define SBMIX_INPUT_MIDI_L_BIT	(1<<6)
#define SBMIX_INPUT_MIDI_R_BIT	(1<<5)
#define SBMIX_INPUT_LINE_L_BIT	(1<<4)
#define SBMIX_INPUT_LINE_R_BIT	(1<<3)
#define SBMIX_INPUT_CD_L_BIT	(1<<2)
#define SBMIX_INPUT_CD_R_BIT	(1<<1)
#define SBMIX_INPUT_MIC_BIT		(1<<0)

enum {SBMIX_LEFT, SBMIX_RIGHT, SBMIX_INPUT, SBMIX_OUTPUT, SBMIX_TREBLE, SBMIX_BASS};

void sbmix_write(uint8_t reg, uint8_t data);
uint8_t sbmix_read(uint8_t reg);
void sbmix_set_vol(uint8_t reg, uint8_t vol);
void sbmix_set_switch(uint8_t io, uint8_t lr, uint8_t bit);
void sbmix_set_gain(uint8_t io, uint8_t lr, uint8_t val);
void sbmix_set_agc(uint8_t val);
void sbmix_set_eq(uint8_t tb, uint8_t lr, uint8_t val);
void sbmix_init(void);

#endif /* SB_MIXER_H_ */
