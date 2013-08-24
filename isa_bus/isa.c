/*
 * isa.c
 *
 *  Created on: 05-07-2013
 *      Author: Polprzewodnikowy
 */

#include "inc/stm32f4xx.h"
#include "gpio.h"
#include "misc/delay.h"
#include "isa_bus/isa.h"

void isa_bus_init(void)
{
	G_DATA->MODER = 0x00000000;		//Input
	G_DATA->OTYPER = 0x0000;		//Push-pull
	G_DATA->OSPEEDR = 0xFFFFFFFF;	//100MHz
	G_DATA->PUPDR = 0x00000000;		//No pull-up/down

	gpio_pin_cfg(G_RESET, P_RESET, GPIO_OUT_PP_100MHz);
	gpio_pin_cfg(G_IOW, P_IOW, GPIO_OUT_PP_100MHz);
	gpio_pin_cfg(G_IOR, P_IOR, GPIO_OUT_PP_100MHz);
	gpio_pin_cfg(G_AEN, P_AEN, GPIO_OUT_PP_100MHz);
	gpio_pin_cfg(G_ALE, P_ALE, GPIO_OUT_PP_100MHz);
	gpio_pin_cfg(G_DIR, P_DIR, GPIO_OUT_PP_100MHz);
	gpio_pin_cfg(G_IOCHRDY, P_IOCHRDY, GPIO_IN_PULL_UP);
//	gpio_pin_cfg(G_IRQ5, P_IRQ5, GPIO_IN_PULL_DOWN);
//	gpio_pin_cfg(G_DRQ5, P_DRQ5, GPIO_IN_FLOATING);
//	gpio_pin_cfg(G_DACK5, P_DACK5, GPIO_OUT_PP_100MHz);
//
//	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PA;	//IRQ5 interrupt configuration / 0|2 = PA2
//	EXTI->PR |= (1<<P_IRQ5);
//	EXTI->RTSR |= (1<<P_IRQ5);
//	EXTI->IMR |= (1<<P_IRQ5);

//	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;	//DRQ5 interrupt configuration / 0|1 = PA1
//	EXTI->PR |= (1<<P_DRQ5);
//	EXTI->RTSR |= (1<<P_DRQ5);
//	EXTI->IMR |= (1<<P_DRQ5);

	RESET_wbb = 1;
	IOW_wbb = 1;
	IOR_wbb = 1;
	ALE_wbb = 0;
	AEN_wbb = 0;
	DIR_wbb = 0;
//	DACK5_wbb = 1;

	delay_ms(20);

	RESET_wbb = 0;

	isa_bus_out(0, 0);
}

void isa_bus_out(uint16_t address, uint16_t data)
{
//	__disable_irq();
	DIR_wbb = 1;
	G_DATA->MODER = 0x55555555;	//Output
	G_DATA->ODR = address;
	ALE_wbb = 1;
	ALE_wbb = 0;
	G_DATA->ODR = data;
	IOW_wbb = 0;
	for(int i = 0; i < 50; i++)
		__asm volatile("nop");
	while(!IOCHRDY_rbb);
	IOW_wbb = 1;
//	__enable_irq();
	return;
}

uint16_t isa_bus_in(uint16_t address)
{
//	__disable_irq();
	DIR_wbb = 1;
	G_DATA->MODER = 0x55555555;	//Output
	G_DATA->ODR = address;
	ALE_wbb = 1;
	ALE_wbb = 0;
	DIR_wbb = 0;
	G_DATA->MODER = 0x00000000;	//Input
	IOR_wbb = 0;
	for(int i = 0; i < 50; i++)
		__asm volatile("nop");
	while(!IOCHRDY_rbb);
	uint16_t data = G_DATA->IDR;
	IOR_wbb = 1;
//	__enable_irq();
	return data;
}
