/*
 * emu8000_synth.c / Created on: 24-12-2012 / Author: Polprzewodnikowy
 */

#include "sound/emu8000/emu8000.h"

void reset_regs(emu8000_reg_t *reg)
{
	reg->start_address = 0;
	reg->loop_start = 0;
	reg->loop_end = 0;
	reg->attenuation = 0x00;
	reg->initial_pitch = 0xE000;
	reg->initial_cutoff = 0xFF;
	reg->pitch_target = 0x4000;
	reg->volume_target = 0x0000;
	reg->cutoff_target = 0xFFFF;
	reg->pitch_height = 0x00;
	reg->cutoff_height = 0x00;
	reg->pan = 0x7F;
	reg->reverb = 0x00;
	reg->chorus = 0x00;
	reg->aux = 0x00;
	reg->filter_q = 0x00;
	reg->volenv_delay = 0x8000;
	reg->volenv_attack = 0x7F;
	reg->volenv_hold = 0x7F;
	reg->volenv_decay = 0x7F;
	reg->volenv_sustain = 0x7F;
	reg->volenv_release = 0x7F;
	reg->modenv_delay = 0x8000;
	reg->modenv_attack = 0x7F;
	reg->modenv_hold = 0x7F;
	reg->modenv_decay = 0x7F;
	reg->modenv_sustain = 0x7F;
	reg->modenv_release = 0x7F;
	reg->lfo1_delay = 0x8000;
	reg->lfo1_frequency = 0x00;
	reg->lfo1_vibrato = 0x00;
	reg->lfo1_cutoff = 0x00;
	reg->lfo1_tremolo = 0x00;
	reg->lfo2_delay = 0x8000;
	reg->lfo2_frequency = 0x00;
	reg->lfo2_vibrato = 0x00;
}

void emu8000_start_voice(uint8_t ch, emu8000_reg_t reg)
{
	/* channel to be silent and idle */
	DCYSUSV_W(ch, 0x0080);
	VTFT_W(ch, 0x0000FFFF);
	CVCF_W(ch, 0x0000FFFF);
	PTRX_W(ch, 0x00000000);
	CPF_W(ch, 0x00000000);

	/* set pitch offset */
	IP_W(ch, reg.initial_pitch);

	/* set envelope parameters */
	ENVVAL_W(ch, reg.modenv_delay);
	ATKHLD_W(ch, (reg.modenv_hold<<8) | (reg.modenv_attack));
	DCYSUS_W(ch, (reg.modenv_sustain<<8) | (reg.modenv_decay));
	ENVVOL_W(ch, reg.volenv_delay);
	ATKHLDV_W(ch, (reg.volenv_hold<<8) | (reg.volenv_attack));
	/* decay/sustain parameter for volume envelope is used for triggerg the voice */

	/* cutoff and volume */
	IFATN_W(ch, (reg.initial_cutoff<<8) | (reg.attenuation));

	/* modulation envelope heights */
	PEFE_W(ch, (reg.pitch_height<<8) | (reg.cutoff_height));

	/* lfo1/2 delay */
	LFO1VAL_W(ch, reg.lfo1_delay);
	LFO2VAL_W(ch, reg.lfo2_delay);

	/* lfo1 pitch & cutoff shift */
	FMMOD_W(ch, (reg.lfo1_vibrato<<8) | (reg.lfo1_cutoff));

	/* lfo1 volume & freq */
	TREMFRQ_W(ch, (reg.lfo1_tremolo<<8) | (reg.lfo1_frequency));

	/* lfo2 pitch & freq */
	FM2FRQ2_W(ch, (reg.lfo2_vibrato<<8) | (reg.lfo2_frequency));

	/* pan & loop start */
	PSST_W(ch, (reg.pan<<24) | (reg.loop_start - 1));

	/* chorus & loop end (chorus 8bit, MSB) */
	CSL_W(ch, (reg.chorus<<24) | (reg.loop_end - 1));

	/* Q & current address (Q 4bit value, MSB) */
	CCCA_W(ch, (reg.filter_q<<28) | (reg.start_address - 1));

	/* clear unknown registers */
	R_00A0_W(ch, 0);
	R_0080_W(ch, 0);

	/* reset volume */
	VTFT_W(ch, (reg.volume_target<<16) | (reg.cutoff_target));
	CVCF_W(ch, (reg.volume_target<<16) | 0xFF00);

	/* trigger voice */
	PTRX_W(ch, (reg.pitch_target<<16) | (reg.reverb<<8) | (reg.aux));
	CPF_W(ch, reg.pitch_target<<16);
	DCYSUSV_W(ch, (reg.volenv_sustain<<8) | (reg.volenv_decay));
}

void emu8000_update_voice(uint8_t ch, emu8000_reg_t reg, int update)
{
	if(update & SET_VOLUME)
		IFATN_W(ch, (reg.initial_cutoff<<8) | (reg.attenuation));
	if(update & SET_PITCH)
		IP_W(ch, reg.initial_pitch);
	if(update & SET_PAN)
		PSST_W(ch, (reg.pan<<24) | (reg.loop_start - 1));
	if(update & SET_FMMOD)
		FMMOD_W(ch, (reg.lfo1_vibrato<<8) | (reg.lfo1_cutoff));
	if(update & SET_TREMFRQ)
		TREMFRQ_W(ch, (reg.lfo1_tremolo<<8) | (reg.lfo1_frequency));
	if(update & SET_FM2FRQ2)
		FM2FRQ2_W(ch, (reg.lfo2_vibrato<<8) | (reg.lfo2_frequency));
	if(update & SET_Q)
		CCCA_W(ch, (reg.filter_q<<28) | (CCCA_R(ch) & 0xFFFFFF));
}

void emu8000_release_voice(uint8_t ch, emu8000_reg_t reg)
{
	DCYSUS_W(ch, (1<<15) | (reg.modenv_release));
	DCYSUSV_W(ch, (1<<15) | (reg.volenv_release));
}

void emu8000_terminate_voice(uint8_t ch)
{
	DCYSUSV_W(ch, 0x807F);
}

void emu8000_reset_voice(uint8_t ch)
{
	/* set all mod/vol envelope shape to minimum */
	DCYSUSV_W(ch, 0x807F);
	ENVVOL_W(ch, 0x8000);
	ENVVAL_W(ch, 0x8000);
	DCYSUS_W(ch, 0x7F7F);
	ATKHLDV_W(ch, 0x7F7F);
	ATKHLD_W(ch, 0x7F7F);
	PEFE_W(ch, 0x0000);		/* mod envelope height to zero */
	LFO1VAL_W(ch, 0x8000);	/* no delay for LFO1 */
	LFO2VAL_W(ch, 0x8000);
	IP_W(ch, 0xE000);		/* no pitch shift */
	IFATN_W(ch, 0xFF00);	/* volume to minimum */
	FMMOD_W(ch, 0x0000);
	TREMFRQ_W(ch, 0x0000);
	FM2FRQ2_W(ch, 0x0000);
}

uint8_t emu8000_find_voice(emu8000_voice_t *voice)
{
	int best;
	uint16_t prevvol;

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		if(voice[i].state == STATE_RELEASED)
			if((CVCF_R(i)>>16 & 0xFFFF) == 0)
				voice[i].state = STATE_END;

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		if(voice[i].state == STATE_PLAYING)
			if(!voice[i].loop)
				if((CCCA_R(i) & 0xFFFFFF) >= voice[i].reg.loop_start - 1)
					voice[i].state = STATE_END;

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		if(voice[i].state == STATE_END)
			return i;

	best = -1;
	prevvol = 0xFFFF;

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		if(voice[i].state == STATE_RELEASED)
			if((CVCF_R(i)>>16 & 0xFFFF) < prevvol)
			{
				best = i;
				prevvol = CVCF_R(i)>>16 & 0xFFFF;
			}
	if(best >= 0)
		return best;

	best = -1;
	prevvol = 0;

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		if(voice[i].state == STATE_PLAYING)
			if(voice[i].reg.attenuation > prevvol)
			{
				best = i;
				prevvol = voice[i].reg.attenuation;
			}
	if(best >= 0)
		return best;

	return 0;
}
