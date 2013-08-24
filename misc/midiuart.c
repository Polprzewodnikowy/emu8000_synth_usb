/*
 * midiuart.c
 *
 *  Created on: 13-08-2013
 *      Author: Polprzewodnikowy
 */

#include <string.h>
#include "inc/stm32f4xx.h"
#include "config.h"
#include "gpio.h"
#include "event.h"
#include "midiuart.h"

volatile uint8_t mubuf[4];
volatile uint8_t mustatus = 0;
volatile int muindex = 0;
volatile int musysex_len = 0;	//TODO: Send SYSEX cmd

void usart_midi_Init(uint32_t speed)
{
	gpio_pin_cfg(GPIOA, 2, GPIO_AF7_PP_100MHz);
	gpio_pin_cfg(GPIOA, 3, GPIO_AF7_PP_100MHz);

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	USART2->BRR = (FREQUENCY / 4) / speed;
	USART2->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
	USART2->CR3 = USART_CR3_DMAT;

	usart_midi_TxByte(0);

	DMA1_Stream6->CR = 0;
	DMA1_Stream6->PAR = (uint32_t)&USART2->DR;
	DMA1_Stream6->CR = DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PL_1 | DMA_SxCR_CHSEL_2;
	DMA1_Stream6->FCR = DMA_SxFCR_DMDIS;

	NVIC_EnableIRQ(USART2_IRQn);
}

void usart_midi_DeInit(void)
{
	RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
	DMA1_Stream6->CR = 0;

	NVIC_DisableIRQ(USART2_IRQn);

	gpio_pin_cfg(GPIOA, 2, GPIO_IN_FLOATING);
	gpio_pin_cfg(GPIOA, 3, GPIO_IN_FLOATING);
}

void usart_midi_TxPacket(uint8_t *buf, uint8_t size)
{
	static uint8_t dmabuf[3];
	static uint8_t status;

	if(DMA1_Stream6->CR & DMA_SxCR_EN)
		while(!(DMA1->HISR & DMA_HISR_TCIF6));

	if(buf[0] == status)
	{
		++buf;
		--size;
	}else{
		status = buf[0];
	}

	memcpy(dmabuf, buf, size);

	DMA1_Stream6->CR &= ~DMA_SxCR_EN;
	DMA1->HIFCR = DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTCIF6;
	DMA1_Stream6->M0AR = (uint32_t)dmabuf;
	DMA1_Stream6->NDTR = (uint32_t)size;
	DMA1_Stream6->CR |= DMA_SxCR_EN;
}

void usart_midi_TxByte(uint8_t data)
{
	USART2->DR = data;
	while((USART2->SR & USART_SR_TC) != USART_SR_TC);
	USART2->SR &= ~USART_SR_TC;
}

void midiuart_process(uint8_t mpdata)
{
	if(mpdata == MIDI_CLOCK)
		return;
	if(mpdata == MIDI_ACTIVE_SENSING)
		return;

	if(mpdata & MIDI_STATUS_MASK)
	{
		musysex_len = muindex;
		mustatus = mpdata;
		muindex = 0;
		if(mustatus != MIDI_SYSEX_END)
		{
			memset((uint8_t *)mubuf, 0, 16);
			return;
		}
	}

	mubuf[muindex++] = mpdata;

	switch(mustatus & 0xF0)
	{
		case MIDI_NOTE_OFF:
		case MIDI_NOTE_ON:
		case MIDI_AFTERTOUCH_POLY:
		case MIDI_CONTROL_CHANGE:
		case MIDI_PITCH_BEND:
			if(muindex == 2)
			{
				event_t ev;
				ev.type = EVENT_TYPE_MIDI;
				ev.buf[0] = mustatus;
				ev.buf[1] = mubuf[0];
				ev.buf[2] = mubuf[1];
				event_put(ev);
				memset((uint8_t *)mubuf, 0, 16);
				muindex = 0;
			}
			break;
		case MIDI_PROGRAM_CHANGE:
		case MIDI_AFTERTOUCH_CHAN:
			if(muindex == 1)
			{
				event_t ev;
				ev.type = EVENT_TYPE_MIDI;
				ev.buf[0] = mustatus;
				ev.buf[1] = mubuf[0];
				event_put(ev);
				memset((uint8_t *)mubuf, 0, 16);
				muindex = 0;
			}
			break;
		case 0xF0:
			switch(mustatus)
			{
				case MIDI_SYSEX:
					break;
				case MIDI_SYSEX_END:
					memset((uint8_t *)mubuf, 0, 16);
					muindex = 0;
					break;
				default:
					memset((uint8_t *)mubuf, 0, 16);
					muindex = 0;
					break;
			}
			break;
		default:
			memset((uint8_t *)mubuf, 0, 16);
			muindex = 0;
			break;
	}
}

void USART2_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(USART2_IRQn);
	midiuart_process(USART2->DR);
	USART2->SR &= ~USART_SR_RXNE;
}
