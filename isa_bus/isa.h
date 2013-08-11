/*
 * isa.h
 *
 *  Created on: 05-07-2013
 *      Author: Polprzewodnikowy
 */

#ifndef ISA_H_
#define ISA_H_

#include <inttypes.h>
#include "hdr/hdr_bitband.h"

#define G_DATA			GPIOE

#define	G_RESET			GPIOC
#define	P_RESET			15

#define	G_IOW			GPIOB
#define	P_IOW			0

#define	G_IOR			GPIOB
#define	P_IOR			1

#define	G_AEN			GPIOA
#define	P_AEN			15

#define	G_ALE			GPIOC
#define	P_ALE			5

#define G_DIR			GPIOA
#define P_DIR			8

#define	G_IOCHRDY		GPIOC
#define	P_IOCHRDY		11

#define	G_IRQ5			GPIOA
#define	P_IRQ5			2

#define	G_DRQ5			GPIOA
#define	P_DRQ5			1

#define	G_DACK5			GPIOA
#define	P_DACK5			3

#define RESET_wbb		bitband_t m_BITBAND_PERIPH(&G_RESET->ODR, P_RESET)
#define IOW_wbb			bitband_t m_BITBAND_PERIPH(&G_IOW->ODR, P_IOW)
#define IOR_wbb			bitband_t m_BITBAND_PERIPH(&G_IOR->ODR, P_IOR)
#define AEN_wbb			bitband_t m_BITBAND_PERIPH(&G_AEN->ODR, P_AEN)
#define ALE_wbb			bitband_t m_BITBAND_PERIPH(&G_ALE->ODR, P_ALE)
#define DIR_wbb			bitband_t m_BITBAND_PERIPH(&G_DIR->ODR, P_DIR)
#define IOCHRDY_rbb		bitband_t m_BITBAND_PERIPH(&G_IOCHRDY->IDR, P_IOCHRDY)
#define DACK5_wbb		bitband_t m_BITBAND_PERIPH(&G_DACK5->ODR, P_DACK5)

void isa_bus_init(void);
void isa_bus_out(uint16_t address, uint16_t data);
uint16_t isa_bus_in(uint16_t address);

#endif /* ISA_H_ */
