/** \file main.c
 * \brief Sample STM32F4 project
 * \details This file holds a very basic code for STM32F407VG. This code
 * enables the FPU, all GPIO ports, configures Flash wait-states and enables
 * the PLL to achieve the highest allowed frequency for STM32F407VG (168MHz).
 * Main code block just blinks the LED. The LED port and pin are defined in
 * config.h file. Target core frequency and quartz crystal resonator
 * frequency are defined there as well.
 *
 * \author Freddie Chopin, http://www.freddiechopin.info/
 * \date 2012-03-23
 */

/******************************************************************************
* project: emu8000_synth
* chip: STM32F407VG
* compiler: arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 4.6.2
* 	20110921 (release) [ARM/embedded-4_6-branch revision 182083]
*
* prefix: (none)
*
* available global functions:
* 	int main(void)
*
* available local functions:
* 	static void flash_latency(uint32_t frequency)
* 	static void fpu_enable(void)
* 	static uint32_t pll_start(uint32_t crystal, uint32_t frequency)
* 	static void system_init(void)
*
* available interrupt handlers:
******************************************************************************/

/*
+=============================================================================+
| includes
+=============================================================================+
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "inc/stm32f4xx.h"

#include "config.h"

#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"

#include "gpio.h"
#include "event.h"

#include "misc/delay.h"
#include "misc/keyb.h"
#include "misc/spi.h"
#include "misc/led.h"
#include "misc/midiuart.h"
#include "misc/usbcontrol.h"

#include "fatfs/ff.h"
#include "fatfs/diskio.h"

#include "sound/sb/sb.h"
#include "sound/emu8000/emu8000.h"
#include "sound/emu8000/soundfont.h"

#include <usbd_dev.h>
#include <usbd_desc.h>
#include <usbd_usr.h>
#include <usb_bsp.h>

/*
+=============================================================================+
| module variables
+=============================================================================+
*/

extern USB_OTG_CORE_HANDLE USB_Dev;
//extern midi_ch_t channel[];
//extern emu8000_voice_t voice[];
SFInfo sf;

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/

static void flash_latency(uint32_t frequency);
static void fpu_enable(void);
static uint32_t pll_start(uint32_t crystal, uint32_t frequency);
static void system_init(void);

/*
+=============================================================================+
| global functions
+=============================================================================+
*/

/*------------------------------------------------------------------------*//**
* \brief main code block
* \details Call some static initialization functions and blink the led with
* frequency defined via count_max variable.
*//*-------------------------------------------------------------------------*/

int main(void)
{
	FATFS fatfs;
	event_t *ev;

	fpu_enable();
	system_init();
	pll_start(CRYSTAL, FREQUENCY);
	SysTick_Config(FREQUENCY / 1000);
	f_mount(0, &fatfs);

	sb_init();
	load_soundfont(&sf, "SF/4GMGS.SF2");
	reg_soundfont(&sf);
	bridge_init();
	usart_midi_Init(31250);

	USBD_Init(&USB_Dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_cb, &USR_cb);

	while(1)
	{
		if(GetKeys() == KEY1)
			emu8000_load_samples(&sf, "SF/4GMGS.SF2");

		if(event_any())
		{
			ev = event_get();
			switch(ev->type)
			{
				case EVENT_TYPE_MIDI:
					switch(ev->buf[0] & 0xF0)
					{
						case MIDI_NOTE_OFF:
							note_off(ev->buf[0] & 0x0F, ev->buf[1], ev->buf[2]);
							break;
						case MIDI_NOTE_ON:
							note_on(ev->buf[0] & 0x0F, ev->buf[1], ev->buf[2]);
							break;
						case MIDI_AFTERTOUCH_POLY:
							aftertouch_poly(ev->buf[0] & 0x0F, ev->buf[1], ev->buf[2]);
							break;
						case MIDI_CONTROL_CHANGE:
							control_change(ev->buf[0] & 0x0F, ev->buf[1], ev->buf[2]);
							break;
						case MIDI_PITCH_BEND:
							pitch_bend(ev->buf[0] & 0x0F, ev->buf[1], ev->buf[2]);
							break;
						case MIDI_PROGRAM_CHANGE:
							program_change(ev->buf[0] & 0x0F, ev->buf[1]);
							break;
						case MIDI_AFTERTOUCH_CHAN:
							aftertouch_chan(ev->buf[0] & 0x0F, ev->buf[1]);
							break;
					}
					break;
				case EVENT_TYPE_USB:
					usbcontrol_process_data(ev->buf);
					break;
				case EVENT_TYPE_NULL:
				default:
					break;
			}
		}
	}
}

/*
+=============================================================================+
| local functions
+=============================================================================+
*/

/*------------------------------------------------------------------------*//**
* \brief Configures Flash latency.
* \details Configures Flash latency (wait-states) which allows the chip to run
* at higher speeds. The constants used for calculations are valid when chip's
* voltage is in 2.7V to 3.6V range.
*
* \param [in] frequency defines the target frequency of the core
*//*-------------------------------------------------------------------------*/

static void flash_latency(uint32_t frequency)
{
	uint32_t wait_states;

	wait_states = frequency / 30000000ul;	// calculate wait_states (30M is valid for 2.7V to 3.6V voltage range, use 24M for 2.4V to 2.7V, 18M for 2.1V to 2.4V or 16M for  1.8V to 2.1V)
	wait_states &= 7;						// trim to max allowed value - 7

	FLASH->ACR = wait_states;				// set wait_states, disable all caches and prefetch
	FLASH->ACR = FLASH_ACR_DCRST | FLASH_ACR_ICRST | wait_states;	// reset caches
	FLASH->ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | wait_states;	// enable caches and prefetch
}

/*------------------------------------------------------------------------*//**
* \brief Enables FPU
* \details Enables FPU in Cortex-M4 for both privileged and user mode. This is
* done by enabling CP10 and CP11 coprocessors in CPACR register (possible only
* when in privileged mode).
*//*-------------------------------------------------------------------------*/

static void fpu_enable(void)
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));	// set CP10 and CP11 Full Access
#endif
}

/*------------------------------------------------------------------------*//**
* \brief Starts the PLL
* \details Configure and enable PLL to achieve some frequency with some crystal.
* Before the speed change Flash latency is configured via flash_latency(). PLL
* parameters are based on both function parameters. The PLL is set up,
* started and connected. AHB clock ratio is set to 1:1 with core clock, APB1 -
* 1:4 (max 42MHz), APB2 - 1:2 (max 84MHz). USB OTG, RNG and SDIO clock is set
* to (VCO output / 9) (max 48MHz).
*
* \param [in] crystal is the frequency of the crystal resonator connected to the
* STM32F407VG
* \param [in] frequency is the desired target frequency after enabling the PLL
*
* \return real frequency that was set
*//*-------------------------------------------------------------------------*/

static uint32_t pll_start(uint32_t crystal, uint32_t frequency)
{
	uint32_t div, mul, div_core, vco_input_frequency, vco_output_frequency, frequency_core;
	uint32_t best_div = 0, best_mul = 0, best_div_core = 0, best_frequency_core = 0;

	RCC_CR_HSEON_bb = 1;					// enable HSE clock
	flash_latency(frequency);				// configure Flash latency for desired frequency

	for (div = 2; div <= 63; div++)			// PLLM in [2; 63]
	{
		vco_input_frequency = crystal / div;

		if ((vco_input_frequency < 1000000ul) || (vco_input_frequency > 2000000))	// skip invalid settings
			continue;

		for (mul = 64; mul <= 432; mul++)	// PLLN in [64; 432]
		{
			vco_output_frequency = vco_input_frequency * mul;

			if ((vco_output_frequency < 64000000ul) || (vco_output_frequency > 432000000ul))	// skip invalid settings
				continue;

			for (div_core = 2; div_core <= 8; div_core += 2)	// PLLP in {2, 4, 6, 8}
			{
				frequency_core = vco_output_frequency / div_core;

				if (frequency_core > frequency)	// skip values over desired frequency
					continue;

				if (frequency_core > best_frequency_core)	// is this configuration better than previous one?
				{
					best_frequency_core = frequency_core;	// yes - save values
					best_div = div;
					best_mul = mul;
					best_div_core = div_core;
				}
			}
		}
	}

	RCC->PLLCFGR = (best_div << RCC_PLLCFGR_PLLM_bit) | (best_mul << RCC_PLLCFGR_PLLN_bit) | ((best_div_core / 2 - 1) << RCC_PLLCFGR_PLLP_bit) | RCC_PLLCFGR_PLLQ_DIV7 | RCC_PLLCFGR_PLLSRC_HSE;	// configure PLL factors, always divide USB clock by 9

	RCC->CFGR = RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_HPRE_DIV1;	// AHB - no prescaler, APB1 - divide by 4, APB2 - divide by 2

	while (!RCC_CR_HSERDY_bb);				// wait for stable clock

	RCC_CR_PLLON_bb = 1;					// enable PLL
	while (!RCC_CR_PLLRDY_bb);				// wait for PLL lock

	RCC->CFGR |= RCC_CFGR_SW_PLL;			// change SYSCLK to PLL
	while (((RCC->CFGR) & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// wait for switch

	return best_frequency_core;
}

/*------------------------------------------------------------------------*//**
* \brief Initializes system
* \details Enables all GPIO ports
*//*-------------------------------------------------------------------------*/

static void system_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	gpio_init();
	acc_off();	//Turn off interrupt lines in LIS302DL
	delay_init();
	gpio_pin_cfg(GPIOA, 0, GPIO_IN_FLOATING);		//User key
	gpio_pin_cfg(GPIOD, 12, GPIO_OUT_PP_100MHz);	//Green LED
	gpio_pin_cfg(GPIOD, 13, GPIO_OUT_PP_100MHz);	//Orange LED
	gpio_pin_cfg(GPIOD, 14, GPIO_OUT_PP_100MHz);	//Red LED
	gpio_pin_cfg(GPIOD, 15, GPIO_OUT_PP_100MHz);	//Blue LED
	led_reg_ptr(LED_G, &LEDG_wbb);
	led_reg_ptr(LED_O, &LEDO_wbb);
	led_reg_ptr(LED_R, &LEDR_wbb);
	led_reg_ptr(LED_B, &LEDB_wbb);
}

/*
+=============================================================================+
| ISRs
+=============================================================================+
*/

void SysTick_Handler(void)
{
	static int timer = 0;
	if(timer++ >= 10)
	{
		disk_timerproc();
		KeybProc();
		timer = 0;
	}
	led_process();
}

/******************************************************************************
* END OF FILE
******************************************************************************/
