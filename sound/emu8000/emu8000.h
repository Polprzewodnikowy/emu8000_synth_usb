/*
 * emu8000.h / Created on: 21-12-2012 / Author: Polprzewodnikowy
 */

#ifndef EMU8000_H_
#define EMU8000_H_

#include <inttypes.h>
#include "sound/emu8000/emu8000_midi_bridge.h"
#include "sound/emu8000/emu8000_synth.h"
#include "sound/emu8000/emu8000_reg.h"
#include "sound/emu8000/soundfont.h"

#define EMU8000_DATA0				0x0620
#define EMU8000_DATA1				0x0A20
#define EMU8000_DATA2				0x0A22
#define EMU8000_DATA3				0x0E20
#define EMU8000_POINTER				0x0E22

#define EMU8000_UNIQUE_ID1			0xA5B9
#define EMU8000_UNIQUE_ID2			0x9D53
#define EMU8000_MAX_DRAM 			(28 * 1024 * 1024)	//Max on-board mem is 28MB ???
#define EMU8000_DRAM_OFFSET 		0x200000			//Beginning of on board ram
#define EMU8000_CHANNELS			32					//Number of hardware channels
#define EMU8000_DRAM_VOICES			30					//Number of normal voices

#define EMU8000_RAM_READ			0		//Flags to set a dma channel to read or write
#define EMU8000_RAM_WRITE			1
#define EMU8000_RAM_CLOSE			2
#define EMU8000_RAM_MODE_MASK		0x03
#define EMU8000_RAM_RIGHT			0x10	//Use "right" DMA channel

#define EMU8000_CHORUS_1			0
#define	EMU8000_CHORUS_2			1
#define	EMU8000_CHORUS_3			2
#define	EMU8000_CHORUS_4			3
#define	EMU8000_CHORUS_FEEDBACK		4
#define	EMU8000_CHORUS_FLANGER		5
#define	EMU8000_CHORUS_SHORTDELAY	6
#define	EMU8000_CHORUS_SHORTDELAY2	7
#define EMU8000_CHORUS_PREDEFINED	8
#define EMU8000_CHORUS_NUMBERS		32	//User can define chorus modes up to 32

#define	EMU8000_REVERB_ROOM1		0
#define EMU8000_REVERB_ROOM2		1
#define	EMU8000_REVERB_ROOM3		2
#define	EMU8000_REVERB_HALL1		3
#define	EMU8000_REVERB_HALL2		4
#define	EMU8000_REVERB_PLATE		5
#define	EMU8000_REVERB_DELAY		6
#define	EMU8000_REVERB_PANNINGDELAY	7
#define EMU8000_REVERB_PREDEFINED	8
#define EMU8000_REVERB_NUMBERS		32	//User can define reverb modes up to 32

struct soundfont_chorus_fx {
	uint16_t feedback;		//Feedback level (0xE600-0xE6FF)
	uint16_t delay_offset;	//Delay (0-0x0DA3) [1/44100 sec]
	uint16_t lfo_depth;		//LFO depth (0xBC00-0xBCFF)
	uint32_t delay;			//Right delay (0-0xFFFFFFFF) [1/256/44100 sec]
	uint32_t lfo_freq;		//LFO freq LFO freq (0-0xFFFFFFFF)
};

struct soundfont_reverb_fx {
	uint16_t parms[28];
};

void emu8000_reg_write_word(uint8_t reg, uint8_t channel, uint16_t address, uint16_t data);
void emu8000_reg_write_dword(uint8_t reg, uint8_t channel, uint16_t address, uint32_t data);
uint16_t emu8000_reg_read_word(uint8_t reg, uint8_t channel, uint16_t address);
uint32_t emu8000_reg_read_dword(uint8_t reg, uint8_t channel, uint16_t address);
int emu8000_detect(void);
int emu8000_load_samples(SFInfo *sf, char *path);
void emu8000_dma_chan(int ch, int mode);
void emu8000_SMALR_wait(void);
void emu8000_SMALW_wait(void);
void emu8000_SMARR_wait(void);
void emu8000_SMARW_wait(void);
void emu8000_dram_fm_init(void);
int emu8000_dram_check(void);
int emu8000_init(void);
void emu8000_update_equalizer(int bass, int treble);
void emu8000_update_chorus_mode(int effect);
void emu8000_update_reverb_mode(int effect);

#endif /* EMU8000_H_ */
