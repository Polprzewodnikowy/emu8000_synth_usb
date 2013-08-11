/*
 * sb_pcm.h / Created on: 25-12-2012 / Author: Polprzewodnikowy
 */

#ifndef SB_PCM_H_
#define SB_PCM_H_

#include <inttypes.h>

#define SBSND_RESET			(SB_BASE_ADDRESS | 0x06)
#define SBSND_READ			(SB_BASE_ADDRESS | 0x0A)
#define SBSND_WRITE			(SB_BASE_ADDRESS | 0x0C)
#define SBSND_WRITE_STATUS	(SB_BASE_ADDRESS | 0x0C)
#define SBSND_READ_STATUS	(SB_BASE_ADDRESS | 0x0E)

void sbsnd_init(void);
void sbsnd_reset(void);
void sbsnd_config_audio(uint8_t depth, uint8_t channels, uint16_t sampling, uint32_t block_size);
void sbsnd_close_audio(void);
void sbsnd_write(uint8_t data);
uint8_t sbsnd_read(void);

#endif /* SB_PCM_H_ */
