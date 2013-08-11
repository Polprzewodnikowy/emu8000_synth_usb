/*
 * delay.h
 *
 *  Created on: 2010-10-07
 *      Author: Jakub £asiñski (j.lasinsk@gmail.com)
 */

#ifndef __delay_h
#define __delay_h

#include <inttypes.h>

#define TIMER_MAX UINT16_MAX

void delay_init(void);
void delay_us(uint32_t delayUS);
void delay_ms(uint32_t delayMS);

#endif /*__delay_h*/
