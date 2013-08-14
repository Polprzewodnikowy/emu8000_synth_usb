/*
 * soundfont.h
 *
 *  Created on: 17-07-2013
 *      Author: Polprzewodnikowy
 *      Source: https://github.com/alsaplayer/alsaplayer/tree/master/attic/midi
 */

#ifndef SOUNDFONT_H_
#define SOUNDFONT_H_

#include <inttypes.h>
#include "fatfs/ff.h"

enum {
	Start_Address = 0,
	End_Address,
	Start_Loop_Address,
	End_Loop_Address,
	Start_Address_Coarse,
	LFO1_To_Pitch,
	LFO2_To_Pitch,
	Env1_To_Pitch,
	Initial_Filter_Cutoff,
	Initial_Filter_Q,
	LFO1_To_Filter_Cutoff,
	Env1_To_Filter_Cutoff,
	End_Address_Coarse,
	LFO1_To_Volume,
	Env2_To_Volume,	//u1
	Chorus_Send,
	Reverb_Send,
	Pan_Send,
	Aux_Send,		//u2
	Sample_Volume,	//u3
	RESERVED1,		//u4
	LFO1_Delay,
	LFO1_Freq,
	LFO2_Delay,
	LFO2_Freq,
	Env1_Delay,
	Env1_Attack,
	Env1_Hold,
	Env1_Decay,
	Env1_Sustain,
	Env1_Release,
	Env1_Keynum_To_Hold,
	Env1_Keynum_To_Decay,
	Env2_Delay,
	Env2_Attack,
	Env2_Hold,
	Env2_Decay,
	Env2_Sustain,
	Env2_Release,
	Env2_Keynum_To_Hold,
	Env2_Keynum_To_Decay,
	Instrument_ID,
	RESERVED2,		//r1
	Key_Range,
	Vel_Range,
	Start_Loop_Address_Coarse,
	Keynum,
	Velocity,
	Initial_Attenuation,
	Key_Tuning,		//r2
	End_Loop_Address_Coarse,
	Coarse_Tune,
	Fine_Tune,
	Sample_ID,
	Sample_Flags,
	Sample_Pitch,	//r3
	Scale_Tuning,
	Exclusive_Class,
	Root_Key,
	RESERVED3,		//u5
	End_Oper
};

typedef struct __attribute__ ((packed)) {
	char id[4];
	uint32_t size;
} chunk_t;

typedef struct __attribute__ ((packed)) {
	uint16_t preset;
	uint16_t bank;
	uint16_t bagNdx;
} presethdr_t;

typedef struct __attribute__ ((packed)) {
	uint32_t startsample, endsample;
	uint32_t startloop, endloop;
	uint32_t samplerate;
	uint8_t originalPitch;
	uint8_t pitchCorrection;
	uint16_t sampletype;
} sampleinfo_t;

typedef struct __attribute__ ((packed)) {
	uint16_t bagNdx;
} insthdr_t;

typedef struct __attribute__ ((packed)) {
	uint8_t oper;
	uint16_t amount;
} genrec_t;

typedef struct __attribute__ ((packed)) {
	int nrinfos;
	sampleinfo_t *sampleinfo;

	uint32_t samplepos;
	uint32_t samplesize;

	int nrinsts;
	insthdr_t *insthdr;
	int nribags;
	uint16_t *instbag;
	int nrigens;
	genrec_t *instgen;

	int nrpresets;
	presethdr_t *presethdr;
	int nrpbags;
	uint16_t *presetbag;
	int nrpgens;
	genrec_t *presetgen;
} SFInfo;

int open_soundfont(SFInfo *sf, char *path);
void load_soundfont(FIL *fp, SFInfo *sf);
void free_soundfont(SFInfo *sf);

#endif /* SOUNDFONT_H_ */
