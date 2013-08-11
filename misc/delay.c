/*
 * delay.c
 *
 *  Created on: 2010-10-07
 *      Author: Jakub £asiñski (j.lasinsk@gmail.com)
 */

#include "delay.h"
#include "config.h"
#include "stm32f4xx.h"

static void delay_ticks(uint32_t ticks)
{
	TIM3->CR1 = 0;							//Reset CR1
	TIM3->EGR = TIM_EGR_UG;					//Generate update event
	TIM3->SR = 0;							//Clear status register
	TIM3->CNT = ticks;						//Write to counter register
	TIM3->CR1 = TIM_CR1_DIR | TIM_CR1_CEN;	//Start timer, downcount mode
	while(!(TIM3->SR & TIM_SR_UIF));		//Wait until delay time has elapsed
	return;
}

void delay_init()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
}

void delay_us(uint32_t delayUS)
{
	TIM3->PSC = (((FREQUENCY / 2) / 1000000) - 1);
	//Set prescaler to zero equivalent of one timer tick per 1us
	if(delayUS > TIMER_MAX)
	{
		uint32_t chunkCount = delayUS / TIMER_MAX;
		uint32_t remainder = delayUS % TIMER_MAX;
		uint32_t i;
		for(i = 0; i < chunkCount; i++)
		{
			delay_ticks(TIMER_MAX);
		}
		delay_ticks(remainder);
	}else{
		delay_ticks(delayUS);
	}
}

void delay_ms(uint32_t delayMS)
{
	//Set prescaler to zero equivalent of one timer tick per 0,1ms
	//(1ms would be to much for 16-bit timer and max clock frequency)
	TIM3->PSC = (((FREQUENCY / 2) / 10000) - 1);
	delayMS *= 10;
	if(delayMS > TIMER_MAX)
	{
		uint32_t chunkCount = delayMS / TIMER_MAX;
		uint32_t remainder = delayMS % TIMER_MAX;
		uint32_t i;
		for(i = 0; i < chunkCount; i++)
		{
			delay_ticks(TIMER_MAX);
		}
		delay_ticks(remainder);
	}else{
		delay_ticks(delayMS);
	}
}
