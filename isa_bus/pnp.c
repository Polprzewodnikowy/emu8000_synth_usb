/*
 * pnp.c
 *
 *  Created on: 05-07-2013
 *      Author: Polprzewodnikowy
 */

#include "isa_bus/isa.h"
#include "misc/delay.h"
#include "isa_bus/pnp.h"

void pnp_config_awe64(void)
{
	uint8_t lfsr, seq_55, seq_aa;
	uint32_t low_s = 0, high_s = 0, checksum_s = 0;

	isa_bus_out(PNP_ADDRESS, 0x00);	//Reset the LFSR
	isa_bus_out(PNP_ADDRESS, 0x00);

	isa_bus_out(PNP_ADDRESS, lfsr = 0x6A);

	for(int i = 1; i < 32; i++)
		isa_bus_out(PNP_ADDRESS, lfsr = ((lfsr>>1)|(((lfsr&1)^((lfsr>>1)&1))<<7)));

	isa_bus_out(PNP_ADDRESS, PNP_CONFIG_CONTROL_REG);
	isa_bus_out(PNP_WRITE, PNP_CNF_RESET_CONF_BIT | PNP_CNF_RESET_CSN_BIT);
	isa_bus_out(PNP_ADDRESS, PNP_WAKE_REG);
	isa_bus_out(PNP_WRITE, 0);	//Wake[0]
	isa_bus_out(PNP_ADDRESS, PNP_RD_DATA_REG);
	isa_bus_out(PNP_WRITE, (PNP_READ>>2));	//Set READ_PORT
	isa_bus_out(PNP_ADDRESS, PNP_ISOLATION_REG);

	seq_55 = isa_bus_in(PNP_READ); delay_us(1000);	//Isolate card
	seq_aa = isa_bus_in(PNP_READ); delay_us(1000);

	if(((seq_55 & 0x03) == 0x01) && ((seq_aa & 0x03) == 0x02))
		low_s |= (1<<0);

	for(int i = 1; i < 72; i++)
	{
		seq_55 = isa_bus_in(PNP_READ); delay_us(250);
		seq_aa = isa_bus_in(PNP_READ); delay_us(250);
		if(((seq_55 & 0x03) == 0x01) && ((seq_aa & 0x03) == 0x02))
		{
			if(i<32)
				low_s |= (1<<i);
			else if(i<64)
				high_s |= (1<<(i-32));
			else if(i<72)
				checksum_s |= (1<<(i-64));
		}
	}

	if(!(low_s || high_s || checksum_s))
	{
		//NVIC_SystemReset();	//Reset system if card isn't isolated
		while(1);	//Nope, just halt
	}

	isa_bus_out(PNP_ADDRESS, PNP_CSN_REG);
	isa_bus_out(PNP_WRITE, 1);	//Set CSN to 1

	//---------- SB AUDIO ----------//

	isa_bus_out(PNP_ADDRESS, PNP_LOGICAL_DEV_REG);
	isa_bus_out(PNP_WRITE, 0);	//0 SB AUDIO

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE0_REG);
	isa_bus_out(PNP_WRITE, 0x02);	//0x220
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE0_REG + 1);
	isa_bus_out(PNP_WRITE, 0x20);	//0x220

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE1_REG);
	isa_bus_out(PNP_WRITE, 0x03);	//0x330
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE1_REG + 1);
	isa_bus_out(PNP_WRITE, 0x30);	//0x330

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE2_REG);
	isa_bus_out(PNP_WRITE, 0x03);	//0x388
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE2_REG + 1);
	isa_bus_out(PNP_WRITE, 0x88);	//0x388

	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH0_REG);
	isa_bus_out(PNP_WRITE, 5);	//IRQ5
	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH0_REG + 1);
	isa_bus_out(PNP_WRITE, 2);	//Edge trigger, active high

	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH1_REG);
	isa_bus_out(PNP_WRITE, 0);	//Not used

	isa_bus_out(PNP_ADDRESS, PNP_DMA_CH0_REG);
	isa_bus_out(PNP_WRITE, 4);	//DMA low = 4 (this means not used)

	isa_bus_out(PNP_ADDRESS, PNP_DMA_CH1_REG);
	isa_bus_out(PNP_WRITE, 5);	//DMA high = 5

	isa_bus_out(PNP_ADDRESS, PNP_ACTIVATE_REG);
	isa_bus_out(PNP_WRITE, PNP_ACTIVATE_BIT);	//Activate

	//---------- SB GAME PORT ----------//

	isa_bus_out(PNP_ADDRESS, PNP_LOGICAL_DEV_REG);
	isa_bus_out(PNP_WRITE, 1);	//1 SB GAME PORT

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE0_REG);
	isa_bus_out(PNP_WRITE, 0x02);	//0x200
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE0_REG + 1);
	isa_bus_out(PNP_WRITE, 0x00);	//0x200

	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH0_REG);
	isa_bus_out(PNP_WRITE, 0);

	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH1_REG);
	isa_bus_out(PNP_WRITE, 0);

	isa_bus_out(PNP_ADDRESS, PNP_DMA_CH0_REG);
	isa_bus_out(PNP_WRITE, 4);

	isa_bus_out(PNP_ADDRESS, PNP_DMA_CH1_REG);
	isa_bus_out(PNP_WRITE, 4);

	isa_bus_out(PNP_ADDRESS, PNP_ACTIVATE_REG);
	isa_bus_out(PNP_WRITE, PNP_ACTIVATE_BIT);	//Activate

	//---------- SB EMU8000 SYNTH ----------//

	isa_bus_out(PNP_ADDRESS, PNP_LOGICAL_DEV_REG);
	isa_bus_out(PNP_WRITE, 2);	//2 SB EMU8000 SYNTH

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE0_REG);
	isa_bus_out(PNP_WRITE, 0x06);	//0x620
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE0_REG + 1);
	isa_bus_out(PNP_WRITE, 0x20);	//0x620

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE1_REG);
	isa_bus_out(PNP_WRITE, 0x0A);	//0xA20
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE1_REG + 1);
	isa_bus_out(PNP_WRITE, 0x20);	//0xA20

	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE2_REG);
	isa_bus_out(PNP_WRITE, 0x0E);	//0xE20
	isa_bus_out(PNP_ADDRESS, PNP_IO_RANGE2_REG + 1);
	isa_bus_out(PNP_WRITE, 0x20);	//0xE20

	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH0_REG);
	isa_bus_out(PNP_WRITE, 0);

	isa_bus_out(PNP_ADDRESS, PNP_IRQ_CH1_REG);
	isa_bus_out(PNP_WRITE, 0);

	isa_bus_out(PNP_ADDRESS, PNP_DMA_CH0_REG);
	isa_bus_out(PNP_WRITE, 4);

	isa_bus_out(PNP_ADDRESS, PNP_DMA_CH1_REG);
	isa_bus_out(PNP_WRITE, 4);

	isa_bus_out(PNP_ADDRESS, PNP_ACTIVATE_REG);
	isa_bus_out(PNP_WRITE, PNP_ACTIVATE_BIT);	//Activate

	//---------- END CONFIGURATION ----------//

	isa_bus_out(PNP_ADDRESS, PNP_CONFIG_CONTROL_REG);
	isa_bus_out(PNP_WRITE, PNP_CNF_WFK_BIT);	//Enter Wait for key state

	return;
}
