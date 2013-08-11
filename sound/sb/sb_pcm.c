/*
 * sb_pcm.c / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#include "isa_bus/isa.h"
#include "sound/sb/sb.h"
#include "sound/sb/sb_pcm.h"
#include "misc/delay.h"

void sbsnd_init(void)
{
	sbmix_write(0x80, (1<<1));	//IRQ configure, IRQ5
	sbmix_write(0x81, (1<<5));	//DMA configure, DMA5
	sbsnd_reset();
	return;
}

void sbsnd_reset(void)
{
	isa_bus_out(SBSND_RESET, 1);	//Reset sequence
	delay_us(3);
	isa_bus_out(SBSND_RESET, 0);
	delay_us(100);
	int attempts = 1000;
	while(((sbsnd_read() & 0xFF) != 0xAA) && attempts--);
	delay_us(100);
	return;
}

void sbsnd_config_audio(uint8_t depth, uint8_t channels, uint16_t sampling, uint32_t block_size)
{
	if(sampling < 5000)
		sampling = 5000;
	if(sampling > 44100)
		sampling = 44100;
	sbsnd_write(0x41);
	sbsnd_write((sampling & 0xFF00)>>8);
	sbsnd_write(sampling & 0xFF);
	if(depth == 8)
	{
		sbsnd_write(0xC6);
		if(channels == 1)
			sbsnd_write(0x00);
		else if(channels == 2)
			sbsnd_write(0x20);
	}
	else if(depth == 16)
	{
		sbsnd_write(0xB6);	//B4
		if(channels == 1)
			sbsnd_write(0x10);
		else if(channels == 2)
			sbsnd_write(0x30);
		block_size = (block_size>>1);
	}
	sbsnd_write((block_size-1) & 0xFF);
	sbsnd_write(((block_size-1)>>8) & 0xFF);
	return;
}

void sbsnd_close_audio(void)
{
	sbsnd_write(0xDA);
	sbsnd_write(0xD9);
	sbsnd_reset();
	return;
}

void sbsnd_write(uint8_t data)
{
	int attempts = 1000;
	while((isa_bus_in(SBSND_WRITE_STATUS) & (1<<7)) && attempts--);
	if(attempts == 0)
		return;
	isa_bus_out(SBSND_WRITE, data);
	return;
}

uint8_t sbsnd_read(void)
{
	int attempts = 1000;
	while((!(isa_bus_in(SBSND_READ_STATUS) & (1<<7))) && attempts--);
	if(attempts == 0)
		return 0;
	return isa_bus_in(SBSND_READ);
}
