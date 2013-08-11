/*
 * spi.c / Created on: 05-01-2013 / Author: Polprzewodnikowy
 */

#include "misc/spi.h"
#include "inc/stm32f4xx.h"
#include "gpio.h"

void spi1_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	gpio_pin_cfg(L_SPI_CS_GPIO, L_SPI_CS_PIN, GPIO_OUT_PP_50MHz);		//cs
	gpio_pin_cfg(L_SPI_SCK_GPIO, L_SPI_SCK_PIN, GPIO_AF5_PP_50MHz);		//sck
	gpio_pin_cfg(L_SPI_MISO_GPIO, L_SPI_MISO_PIN, GPIO_AF5_PP_50MHz);	//data in
	gpio_pin_cfg(L_SPI_MOSI_GPIO, L_SPI_MOSI_PIN, GPIO_AF5_PP_50MHz);	//data out
	L_SPI_CS_wbb = 1;
	SPI1->CR1 = SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_BR_0 | SPI_CR1_MSTR;
	SPI1->CR1 |= SPI_CR1_SPE;
	while((SPI1->SR & SPI_SR_TXE) == RESET);
	volatile uint16_t dummy = SPI1->DR;
}

void spi1_deinit(void)
{
	gpio_pin_cfg(L_SPI_CS_GPIO, L_SPI_CS_PIN, GPIO_IN_FLOATING);		//cs
	gpio_pin_cfg(L_SPI_SCK_GPIO, L_SPI_SCK_PIN, GPIO_IN_FLOATING);		//sck
	gpio_pin_cfg(L_SPI_MISO_GPIO, L_SPI_MISO_PIN, GPIO_IN_FLOATING);	//data in
	gpio_pin_cfg(L_SPI_MOSI_GPIO, L_SPI_MOSI_PIN, GPIO_IN_FLOATING);	//data out
	SPI1->CR1 = 0;
	RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
}

uint16_t spi1_trx(uint16_t data)
{
	SPI1->DR = data;
	while((SPI1->SR & SPI_SR_RXNE) == RESET);
	return SPI1->DR;
}

void acc_off(void)
{
	spi1_init();

	L_SPI_CS_wbb = 0;
	spi1_trx(0x20);
	spi1_trx(1<<6);
	L_SPI_CS_wbb = 1;

	L_SPI_CS_wbb = 0;
	spi1_trx(0x22);
	spi1_trx(0xFF);
	L_SPI_CS_wbb = 1;

	spi1_deinit();
}
