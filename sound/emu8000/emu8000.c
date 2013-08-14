/*
 * emu8000.c / Created on: 21-12-2012 / Author: Polprzewodnikowy
 */

#include "sound/emu8000/emu8000_arrays.h"
#include "isa_bus/isa.h"
#include "fatfs/ff.h"
#include "misc/delay.h"
#include "misc/led.h"
#include "sound/emu8000/emu8000.h"

void emu8000_reg_write_word(uint8_t reg, uint8_t channel, uint16_t address, uint16_t data)
{
	isa_bus_out(EMU8000_POINTER, EMU8000_CMD(reg, channel));
	isa_bus_out(address, data);
}

void emu8000_reg_write_dword(uint8_t reg, uint8_t channel, uint16_t address, uint32_t data)
{
	isa_bus_out(EMU8000_POINTER, EMU8000_CMD(reg, channel));
	isa_bus_out(address, data & 0xFFFF);
	isa_bus_out(address + 2, (data>>16) & 0xFFFF);
}

uint16_t emu8000_reg_read_word(uint8_t reg, uint8_t channel, uint16_t address)
{
	uint16_t data = 0;
	isa_bus_out(EMU8000_POINTER, EMU8000_CMD(reg, channel));
	data = isa_bus_in(address);
	return data;
}

uint32_t emu8000_reg_read_dword(uint8_t reg, uint8_t channel, uint16_t address)
{
	uint32_t data = 0;
	isa_bus_out(EMU8000_POINTER, EMU8000_CMD(reg, channel));
	data = isa_bus_in(address);
	data |= (isa_bus_in(address + 2)<<16);
	return data;
}

inline void emu8000_SMALR_wait(void) {while((SMALR_R & 0x80000000) != 0);}
inline void emu8000_SMALW_wait(void) {while((SMALW_R & 0x80000000) != 0);}
inline void emu8000_SMARR_wait(void) {while((SMARR_R & 0x80000000) != 0);}
inline void emu8000_SMARW_wait(void) {while((SMARW_R & 0x80000000) != 0);}

int emu8000_detect(void)
{
	HWCF1_W(0x0059);
	HWCF2_W(0x0020);
	HWCF3_W(0x0000);

	for(int i = 0; i < 100; i++)
	{
		if((HWCF1_R & 0x007E) == 0x0058)
			break;
		if(i >= 99)
			return 1;	//Not detected
	}
	for(int i = 0; i < 100; i++)
	{
		if((HWCF2_R & 0x0003) == 0x0003)
			break;
		if(i >= 99)
			return 1;	//Not detected
	}
	return 0;	//Detected
}

int emu8000_load_samples(SFInfo *sf, char *path)
{
	FIL sample_file;
	int16_t sample_buff;
	UINT bytes;

	if(sf->samplesize == 0)
		return 0;

	if(f_open(&sample_file, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
		return 0;

	f_lseek(&sample_file, sf->samplepos);
	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		emu8000_dma_chan(i, EMU8000_RAM_WRITE);
	SMALW_W(EMU8000_DRAM_OFFSET);
	while(f_tell(&sample_file) < sf->samplepos + sf->samplesize)
	{
		f_read(&sample_file, &sample_buff, 2, &bytes);
		emu8000_SMALW_wait();
		SMLD_W(sample_buff);
		led_blink(LED_O, 1);
	}
	emu8000_SMALW_wait();
	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		emu8000_dma_chan(i, EMU8000_RAM_CLOSE);
	f_close(&sample_file);

	return sf->samplesize - sf->samplepos;
}

void emu8000_dma_chan(int ch, int mode)
{
	unsigned right_bit = (mode & EMU8000_RAM_RIGHT) ? 0x01000000 : 0;
	mode &= EMU8000_RAM_MODE_MASK;
	if(mode == EMU8000_RAM_CLOSE)
	{
		CCCA_W(ch, 0);
		DCYSUSV_W(ch, 0x807F);
		return;
	}
	DCYSUSV_W(ch, 0x80);
	VTFT_W(ch, 0);
	CVCF_W(ch, 0);
	PTRX_W(ch, 0x40000000);
	CPF_W(ch, 0x40000000);
	PSST_W(ch, 0);
	CSL_W(ch, 0);
	if(mode == EMU8000_RAM_WRITE)	/* DMA write */
		CCCA_W(ch, 0x06000000 | right_bit);
	else							/* DMA read */
		CCCA_W(ch, 0x04000000 | right_bit);
}

void emu8000_dram_fm_init(void)
{
	//Initialize the last two channels for DRAM refresh and producing
	//	the reverb and chorus effects for Yamaha OPL-3 synthesizer
	//31: FM left channel, 0x00FFFFE0-0x00FFFFE8
	DCYSUSV_W(30, 0x0080);
	PSST_W(30, 0xFFFFFFE0);	//full left
	CSL_W(30, 0x00FFFFE8|(0x00<<24));	//"Chorus depth" / 0x00FFFFE8|(emu->fm_chorus_depth<<24)
	PTRX_W(30, (0x30<<8));				//"Reverb depth" / (emu->fm_reverb_depth<<8)
	CPF_W(30, 0x00008000);				//CPF_W(30, 0x00000000);
	CCCA_W(30, 0x00FFFFE3);
	//32: FM right channel, 0x00FFFFF0-0x00FFFFF8
	DCYSUSV_W(31, 0x0080);
	PSST_W(31, 0x00FFFFF0); //full right
	CSL_W(31, 0x00FFFFF8|(0x00<<24));	//"Chorus depth" / 0x00FFFFF8|(emu->fm_chorus_depth<<24)
	PTRX_W(31, (0x30<<8));				//"Reverb depth" / (emu->fm_reverb_depth<<8)
	CPF_W(31, 0x00008000);
	CCCA_W(31, 0x00FFFFF3);
	emu8000_reg_write_word(1, 30, EMU8000_DATA0, 0x0000);
	while(!(isa_bus_in(EMU8000_POINTER) & 0x1000));
	while((isa_bus_in(EMU8000_POINTER) & 0x1000));
	emu8000_reg_write_word(1, 30, EMU8000_DATA0, 0x4828);
	//This is really odd part...
	isa_bus_out(EMU8000_POINTER, 0x003C);
	isa_bus_out(EMU8000_DATA1, 0x0000);
	//Skew volume & cutoff
	VTFT_W(30, 0x8000FFFF);
	VTFT_W(31, 0x8000FFFF);
	return;
}

int emu8000_dram_check(void)
{
	int i, size = 0, detected_size = 0;

	/* write out a magic number */
	emu8000_dma_chan(0, EMU8000_RAM_WRITE);
	emu8000_dma_chan(1, EMU8000_RAM_READ);
	SMALW_W(EMU8000_DRAM_OFFSET);
	SMLD_W(EMU8000_UNIQUE_ID1);
	emu8000_dram_fm_init(); /* This must really be here and not 2 lines back even */

	while(size < EMU8000_MAX_DRAM)
	{
		size += 512 * 1024; /* increment 512kbytes */

		/* Write a unique data on the test address.
		 * if the address is out of range, the data is written on
		 * 0x200000(=EMU8000_DRAM_OFFSET).  Then the id word is
		 * changed by this data.
		 */
		/*snd_emu8000_dma_chan(emu, 0, EMU8000_RAM_WRITE);*/
		SMALW_W(EMU8000_DRAM_OFFSET + (size>>1));
		SMLD_W(EMU8000_UNIQUE_ID2);
		emu8000_SMALW_wait();

		/*
		 * read the data on the just written DRAM address
		 * if not the same then we have reached the end of ram.
		 */
		/*snd_emu8000_dma_chan(emu, 0, EMU8000_RAM_READ);*/
		SMALR_W(EMU8000_DRAM_OFFSET + (size>>1));
		/*snd_emu8000_read_wait(emu);*/
		SMLD_R; /* discard stale data  */
		if(SMLD_R != EMU8000_UNIQUE_ID2)
			break; /* no memory at this address */

		detected_size = size;

		emu8000_SMALR_wait();

		/*
		 * If it is the same it could be that the address just
		 * wraps back to the beginning; so check to see if the
		 * initial value has been overwritten.
		 */
		SMALR_W(EMU8000_DRAM_OFFSET);
		SMLD_R; /* discard stale data  */
		if(SMLD_R != EMU8000_UNIQUE_ID1)
			break; /* we must have wrapped around */
		emu8000_SMALR_wait();
	}

	/* wait until FULL bit in SMAxW register is false */
	for(i = 0; i < 10000; i++)
	{
		if((SMALW_R & 0x80000000) == 0)
			break;
	}
	emu8000_dma_chan(0, EMU8000_RAM_CLOSE);
	emu8000_dma_chan(1, EMU8000_RAM_CLOSE);

	//printf_("EMU8000: %d Kb on-board memory detected\n", detected_size/1024);

	return detected_size;
}

int emu8000_init(void)
{
	if(emu8000_detect())
		return 0;

	for(int i = 0; i < 32; i++)
		DCYSUSV_W(i, 0x0080);

	for(int i = 0; i < 32; i++)
	{
		ENVVOL_W(i, 0x0000);
		ENVVAL_W(i, 0x0000);
		DCYSUS_W(i, 0x0000);
		ATKHLDV_W(i, 0x0000);
		LFO1VAL_W(i, 0x0000);
		ATKHLD_W(i, 0x0000);
		LFO2VAL_W(i, 0x0000);
		IP_W(i, 0x0000);
		IFATN_W(i, 0x0000);
		PEFE_W(i, 0x0000);
		FMMOD_W(i, 0x0000);
		TREMFRQ_W(i, 0x0000);
		FM2FRQ2_W(i, 0x0000);
		PTRX_W(i, 0x00000000);
		VTFT_W(i, 0x00000000);
		PSST_W(i, 0x00000000);
		CSL_W(i, 0x00000000);
		CCCA_W(i, 0x00000000);
	}

	for(int i = 0; i < 32; i++)
	{
		CPF_W(i, 0x00000000);
		CVCF_W(i, 0x00000000);
	}

	SMALR_W(0x00000000);
	SMARR_W(0x00000000);
	SMALW_W(0x00000000);
	SMARW_W(0x00000000);

	for(int i = 0; i < 32; i++)
	{
		INIT1_W(i, first_init1[i]);
		INIT2_W(i, first_init2[i]);
		INIT3_W(i, first_init3[i]);
		INIT4_W(i, first_init4[i]);
	}

	delay_ms(24);

	for(int i = 0; i < 32; i++)
	{
		INIT1_W(i, second_init1[i]);
		INIT2_W(i, second_init2[i]);
		INIT3_W(i, second_init3[i]);
		INIT4_W(i, second_init4[i]);
	}

	for(int i = 0; i < 32; i++)
	{
		INIT1_W(i, third_init1[i]);
		INIT2_W(i, third_init2[i]);
		INIT3_W(i, third_init3[i]);
		INIT4_W(i, third_init4[i]);
	}

	HWCF4_W(0x00000000);
	HWCF5_W(0x00000083);
	HWCF6_W(0x00008000);

	for(int i = 0; i < 32; i++)
	{
		INIT1_W(i, fourth_init1[i]);
		INIT2_W(i, fourth_init2[i]);
		INIT3_W(i, fourth_init3[i]);
		INIT4_W(i, fourth_init4[i]);
	}

	emu8000_dram_fm_init();

	for(int i = 0; i < EMU8000_DRAM_VOICES; i++)
		DCYSUSV_W(i, 0x807F);

	int dram_size = emu8000_dram_check();

	emu8000_update_equalizer(8, 9);
	emu8000_update_chorus_mode(EMU8000_CHORUS_3);
	emu8000_update_reverb_mode(EMU8000_REVERB_PLATE);

	HWCF3_W(0x0004);

	return dram_size;
}

void emu8000_update_equalizer(int bass, int treble)
{
	unsigned short w;

	if(bass < 0 || bass > 11 || treble < 0 || treble > 11)
		return;
	INIT4_W(0x01, bass_parm[bass][0]);
	INIT4_W(0x11, bass_parm[bass][1]);
	INIT3_W(0x11, treble_parm[treble][0]);
	INIT3_W(0x13, treble_parm[treble][1]);
	INIT3_W(0x1B, treble_parm[treble][2]);
	INIT4_W(0x07, treble_parm[treble][3]);
	INIT4_W(0x0B, treble_parm[treble][4]);
	INIT4_W(0x0D, treble_parm[treble][5]);
	INIT4_W(0x17, treble_parm[treble][6]);
	INIT4_W(0x19, treble_parm[treble][7]);
	w = bass_parm[bass][2] + treble_parm[treble][8];
	INIT4_W(0x15, (unsigned short)(w + 0x0262));
	INIT4_W(0x1D, (unsigned short)(w + 0x8362));
}

void emu8000_update_chorus_mode(int effect)
{
	if(effect < 0 || effect >= EMU8000_CHORUS_NUMBERS || (effect >= EMU8000_CHORUS_PREDEFINED))
		return;
	INIT3_W(0x09, chorus_parm[effect].feedback);
	INIT3_W(0x0C, chorus_parm[effect].delay_offset);
	INIT4_W(0x03, chorus_parm[effect].lfo_depth);
	HWCF4_W(chorus_parm[effect].delay);
	HWCF5_W(chorus_parm[effect].lfo_freq);
	HWCF6_W(0x00008000);
	HWCF7_W(0x00000000);
}

void emu8000_update_reverb_mode(int effect)
{
	if(effect < 0 || effect >= EMU8000_REVERB_NUMBERS || (effect >= EMU8000_REVERB_PREDEFINED))
		return;
	for(int i = 0; i < 28; i++)
		emu8000_reg_write_word(reverb_cmds[i].reg, reverb_cmds[i].ch, reverb_cmds[i].port, reverb_parm[effect].parms[i]);
}
