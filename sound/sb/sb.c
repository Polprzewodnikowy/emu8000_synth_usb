/*
 * sb.c / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#include "inc/stm32f4xx.h"
#include "isa_bus/isa.h"
#include "isa_bus/pnp.h"
#include "sound/sb/sb_mixer.h"
#include "sound/sb/sb_pcm.h"
#include "sound/emu8000/emu8000.h"
#include "sound/sb/sb.h"

void sb_init(void)
{
	isa_bus_init();
	pnp_config_awe64();
	sbsnd_init();
	sbmix_init();
	emu8000_init();
	//NVIC_EnableIRQ(EXTI1_IRQn);	//DRQ
	//NVIC_EnableIRQ(EXTI2_IRQn);	//IRQ
}
