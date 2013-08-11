/*
 * pnp.h
 *
 *  Created on: 05-07-2013
 *      Author: Polprzewodnikowy
 */

#ifndef PNP_H_
#define PNP_H_

#define PNP_ADDRESS 0x279
#define PNP_WRITE 0xA79
#define PNP_READ 0x203	//Bits 0 and 1 must be set to 1

#define PNP_RD_DATA_REG			0x00
#define PNP_ISOLATION_REG		0x01
#define PNP_CONFIG_CONTROL_REG	0x02
#define PNP_WAKE_REG			0x03
#define PNP_RESOUCE_REG			0x04
#define PNP_STATUS_REG			0x05
#define PNP_CSN_REG				0x06
#define PNP_LOGICAL_DEV_REG		0x07
#define PNP_ACTIVATE_REG		0x30
#define PNP_IO_RANGE_CHECK_REG	0x31
#define PNP_24_MEM_DESC0_REG	0x40
#define PNP_24_MEM_DESC1_REG	0x48
#define PNP_24_MEM_DESC2_REG	0x50
#define PNP_24_MEM_DESC3_REG	0x58
#define PNP_IO_RANGE0_REG		0x60
#define PNP_IO_RANGE1_REG		0x62
#define PNP_IO_RANGE2_REG		0x64
#define PNP_IO_RANGE3_REG		0x66
#define PNP_IO_RANGE4_REG		0x68
#define PNP_IO_RANGE5_REG		0x6A
#define PNP_IO_RANGE6_REG		0x6C
#define PNP_IO_RANGE7_REG		0x6E
#define PNP_IRQ_CH0_REG			0x70
#define PNP_IRQ_CH1_REG			0x72
#define PNP_DMA_CH0_REG			0x74
#define PNP_DMA_CH1_REG			0x75
#define PNP_32_MEM_RANGE0_REG	0x76
#define PNP_32_MEM_RANGE1_REG	0x80
#define PNP_32_MEM_RANGE2_REG	0x90
#define PNP_32_MEM_RANGE3_REG	0xA0

#define PNP_ACTIVATE_BIT		(1<<0)
#define PNP_CNF_RESET_CONF_BIT	(1<<0)
#define PNP_CNF_WFK_BIT			(1<<1)
#define PNP_CNF_RESET_CSN_BIT	(1<<2)

#define PNP_SRES_VER 			0x01
#define PNP_SRES_LOG_DEV_ID		0x02
#define PNP_SRES_COMPAT_DEV_ID	0x03
#define PNP_SRES_IRQ_FMT		0x04
#define PNP_SRES_DMA_FMT		0x05
#define PNP_SRES_SDF			0x06
#define PNP_SRES_EDF			0x07
#define PNP_SRES_IO_DESC		0x08
#define PNP_SRES_FIXED_IO_DESC	0x09
#define PNP_SRES_VENDOR_DEFINED	0x0E
#define PNP_SRES_END_TAG		0x0F

#define PNP_LRES_MEM_RANGE_DESC				0x01
#define PNP_LRES_ID_STRING_ANSI				0x02
#define PNP_LRES_ID_STRING_UNICODE			0x03
#define PNP_LRES_VENDOR_DEFINED				0x04
#define PNP_LRES_32_MEM_RANGE_DESC			0x05
#define PNP_LRES_32_FIXED_MEM_RANGE_DESC	0x06

void pnp_config_awe64(void);

#endif /* PNP_H_ */
