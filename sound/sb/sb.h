/*
 * sb.h / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#ifndef SB_H_
#define SB_H_

#include "isa_bus/isa.h"
#include "isa_bus/pnp.h"
#include "sound/sb/sb_pcm.h"
#include "sound/sb/sb_mixer.h"
#include "sound/emu8000/emu8000.h"

#define SB_BASE_ADDRESS 0x220

void sb_init(void);

#endif /* SB_H_ */
