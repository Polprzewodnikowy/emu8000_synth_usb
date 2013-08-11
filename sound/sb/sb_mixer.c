/*
 * sb_mixer.c / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#include "isa_bus/isa.h"
#include "sound/sb/sb.h"
#include "sound/sb/sb_mixer.h"

void sbmix_write(uint8_t reg, uint8_t data)
{
	isa_bus_out(SBMIX_ADDRESS, reg);
	isa_bus_out(SBMIX_DATA, data);
}

uint8_t sbmix_read(uint8_t reg)
{
	isa_bus_out(SBMIX_ADDRESS, reg);
	return isa_bus_in(SBMIX_DATA);
}

void sbmix_set_vol(uint8_t reg, uint8_t vol)
{
	if(reg >= SBMIX_VOICE_LR && reg <= SBMIX_LINE_LR)
		vol = (vol & 0xF0) | (vol>>4);
	else if(reg >= SBMIX_MASTER_L && reg <= SBMIX_MIC)
		vol = (vol & 0xF8);
	else if(reg == SBMIX_PCSPK)
		vol = (vol & 0xC0);
	else
		return;
	sbmix_write(reg, vol);
}

void sbmix_set_switch(uint8_t io, uint8_t lr, uint8_t bit)
{
	if(io == SBMIX_INPUT)
	{
		if(!(bit & 0x80))
		{
			if(lr == SBMIX_LEFT)
				sbmix_write(SBMIX_INPUT_SWITCH_L, bit);
			else if(lr == SBMIX_RIGHT)
				sbmix_write(SBMIX_INPUT_SWITCH_R, bit);
		}

	}
	else if(io == SBMIX_OUTPUT)
	{
		if(!(bit & 0xE0))
		{
			sbmix_write(SBMIX_OUTPUT_SWITCH, bit);
		}
	}
}

void sbmix_set_gain(uint8_t io, uint8_t lr, uint8_t val)
{
	val = ((val & 0x03)<<6);
	if(io == SBMIX_INPUT)
	{
		if(lr == SBMIX_LEFT)
			sbmix_write(SBMIX_INPUT_GAIN_L, val);
		else if(lr == SBMIX_RIGHT)
			sbmix_write(SBMIX_INPUT_GAIN_R, val);
	}
	else if(io == SBMIX_OUTPUT)
	{
		if(lr == SBMIX_LEFT)
			sbmix_write(SBMIX_OUTPUT_GAIN_L, val);
		else if(lr == SBMIX_RIGHT)
			sbmix_write(SBMIX_OUTPUT_GAIN_R, val);
	}
}

void sbmix_set_agc(uint8_t val)
{
	if(val)
		sbmix_write(SBMIX_AGC, 1);
	else
		sbmix_write(SBMIX_AGC, 0);
}

void sbmix_set_eq(uint8_t tb, uint8_t lr, uint8_t val)
{
	val = ((val & 0x0F)<<4);
	if(tb == SBMIX_TREBLE)
	{
		if(lr == SBMIX_LEFT)
			sbmix_write(SBMIX_TREBLE_L, val);
		else if(lr == SBMIX_RIGHT)
			sbmix_write(SBMIX_TREBLE_R, val);
	}
	else if(tb == SBMIX_BASS)
	{
		if(lr == SBMIX_LEFT)
			sbmix_write(SBMIX_BASS_L, val);
		else if(lr == SBMIX_RIGHT)
			sbmix_write(SBMIX_BASS_R, val);
	}
}

void sbmix_init(void)
{
	sbmix_write(SBMIX_RESET, 0xFF);			//Reset mixer
	//sbmix_set_vol(SBMIX_MASTER_L, 0xFF);	//Load default settings
	//sbmix_set_vol(SBMIX_MASTER_R, 0xFF);
	//sbmix_set_vol(SBMIX_VOICE_L, 0xFF);
	//sbmix_set_vol(SBMIX_VOICE_R, 0xFF);
	sbmix_set_vol(SBMIX_MIDI_L, 0xFF);
	sbmix_set_vol(SBMIX_MIDI_R, 0xFF);
	sbmix_set_vol(SBMIX_CD_L, 0x00);
	sbmix_set_vol(SBMIX_CD_R, 0x00);
	sbmix_set_vol(SBMIX_LINE_L, 0x00);
	sbmix_set_vol(SBMIX_LINE_R, 0x00);
	sbmix_set_vol(SBMIX_MIC, 0x00);
	sbmix_set_vol(SBMIX_PCSPK, 0x00);
	sbmix_set_switch(SBMIX_OUTPUT, 0, 0);
	sbmix_set_switch(SBMIX_INPUT, SBMIX_LEFT, 0);
	sbmix_set_switch(SBMIX_INPUT, SBMIX_RIGHT, 0);
	sbmix_set_gain(SBMIX_INPUT, SBMIX_LEFT, 0);
	sbmix_set_gain(SBMIX_INPUT, SBMIX_RIGHT, 0);
	sbmix_set_gain(SBMIX_OUTPUT, SBMIX_LEFT, 0);
	sbmix_set_gain(SBMIX_OUTPUT, SBMIX_RIGHT, 0);
	sbmix_set_agc(0);
	sbmix_set_eq(SBMIX_TREBLE, SBMIX_LEFT, 8);
	sbmix_set_eq(SBMIX_TREBLE, SBMIX_RIGHT, 8);
	sbmix_set_eq(SBMIX_BASS, SBMIX_LEFT, 8);
	sbmix_set_eq(SBMIX_BASS, SBMIX_RIGHT, 8);
	//sbmix_write(SBMIX_3DSE, 1);
}
