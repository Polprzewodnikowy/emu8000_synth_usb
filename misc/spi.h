/*
 * spi.h / Created on: 05-01-2013 / Author: Polprzewodnikowy
 */

#ifndef SPI_H_
#define SPI_H_

#include <inttypes.h>
#include "hdr/hdr_bitband.h"

#define L_SPI_SCK_GPIO	GPIOA
#define L_SPI_MOSI_GPIO	GPIOA
#define L_SPI_MISO_GPIO	GPIOA
#define L_SPI_CS_GPIO	GPIOE

#define L_SPI_SCK_PIN	5
#define L_SPI_MOSI_PIN	7
#define L_SPI_MISO_PIN	6
#define L_SPI_CS_PIN	3

#define L_SPI_CS_wbb	bitband_t m_BITBAND_PERIPH(&L_SPI_CS_GPIO->ODR, L_SPI_CS_PIN)

void spi1_init(void);
void spi1_deinit(void);
uint16_t spi1_trx(uint16_t data);
void acc_off(void);

#endif /* SPI_H_ */
