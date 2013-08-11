/*
 * emu8000_reg.h / Created on: 24-12-2012 / Author: Polprzewodnikowy
 */

#ifndef EMU8000_REG_H_
#define EMU8000_REG_H_

#define EMU8000_CMD(reg, channel)	(((reg<<5) & 0xE0) | (channel & 0x1F))

#define CPF_W(ch, data)		emu8000_reg_write_dword(0, ch, EMU8000_DATA0, data)
#define PTRX_W(ch, data)	emu8000_reg_write_dword(1, ch, EMU8000_DATA0, data)
#define CVCF_W(ch, data)	emu8000_reg_write_dword(2, ch, EMU8000_DATA0, data)
#define VTFT_W(ch, data)	emu8000_reg_write_dword(3, ch, EMU8000_DATA0, data)
#define PSST_W(ch, data)	emu8000_reg_write_dword(6, ch, EMU8000_DATA0, data)
#define CSL_W(ch, data)		emu8000_reg_write_dword(7, ch, EMU8000_DATA0, data)
#define CCCA_W(ch, data)	emu8000_reg_write_dword(0, ch, EMU8000_DATA1, data)
#define HWCF4_W(data)		emu8000_reg_write_dword(1, 9, EMU8000_DATA1, data)
#define HWCF5_W(data)		emu8000_reg_write_dword(1, 10, EMU8000_DATA1, data)
#define HWCF6_W(data)		emu8000_reg_write_dword(1, 13, EMU8000_DATA1, data)
#define HWCF7_W(data)		emu8000_reg_write_dword(1, 14, EMU8000_DATA1, data)	//HWCF7 register
#define SMALR_W(data)		emu8000_reg_write_dword(1, 20, EMU8000_DATA1, data)	//is not documented
#define SMARR_W(data)		emu8000_reg_write_dword(1, 21, EMU8000_DATA1, data)
#define SMALW_W(data)		emu8000_reg_write_dword(1, 22, EMU8000_DATA1, data)
#define SMARW_W(data)		emu8000_reg_write_dword(1, 23, EMU8000_DATA1, data)
#define SMLD_W(data)		emu8000_reg_write_word(1, 26, EMU8000_DATA1, data)
#define SMRD_W(data)		emu8000_reg_write_word(1, 26, EMU8000_DATA2, data)
#define WC_W(data)			emu8000_reg_write_word(1, 27, EMU8000_DATA2, data)
#define HWCF1_W(data)		emu8000_reg_write_word(1, 29, EMU8000_DATA1, data)
#define HWCF2_W(data)		emu8000_reg_write_word(1, 30, EMU8000_DATA1, data)
#define HWCF3_W(data)		emu8000_reg_write_word(1, 31, EMU8000_DATA1, data)
#define INIT1_W(ch, data)	emu8000_reg_write_word(2, ch, EMU8000_DATA1, data)
#define INIT2_W(ch, data)	emu8000_reg_write_word(2, ch, EMU8000_DATA2, data)
#define INIT3_W(ch, data)	emu8000_reg_write_word(3, ch, EMU8000_DATA1, data)
#define INIT4_W(ch, data)	emu8000_reg_write_word(3, ch, EMU8000_DATA2, data)
#define ENVVOL_W(ch, data)	emu8000_reg_write_word(4, ch, EMU8000_DATA1, data)
#define DCYSUSV_W(ch, data)	emu8000_reg_write_word(5, ch, EMU8000_DATA1, data)
#define ENVVAL_W(ch, data)	emu8000_reg_write_word(6, ch, EMU8000_DATA1, data)
#define DCYSUS_W(ch, data)	emu8000_reg_write_word(7, ch, EMU8000_DATA1, data)
#define ATKHLDV_W(ch, data)	emu8000_reg_write_word(4, ch, EMU8000_DATA2, data)
#define LFO1VAL_W(ch, data)	emu8000_reg_write_word(5, ch, EMU8000_DATA2, data)
#define ATKHLD_W(ch, data)	emu8000_reg_write_word(6, ch, EMU8000_DATA2, data)
#define LFO2VAL_W(ch, data)	emu8000_reg_write_word(7, ch, EMU8000_DATA2, data)
#define IP_W(ch, data)		emu8000_reg_write_word(0, ch, EMU8000_DATA3, data)
#define IFATN_W(ch, data)	emu8000_reg_write_word(1, ch, EMU8000_DATA3, data)
#define PEFE_W(ch, data)	emu8000_reg_write_word(2, ch, EMU8000_DATA3, data)
#define FMMOD_W(ch, data)	emu8000_reg_write_word(3, ch, EMU8000_DATA3, data)
#define TREMFRQ_W(ch, data)	emu8000_reg_write_word(4, ch, EMU8000_DATA3, data)
#define FM2FRQ2_W(ch, data)	emu8000_reg_write_word(5, ch, EMU8000_DATA3, data)
#define R_0080_W(ch, data)	emu8000_reg_write_dword(4, ch, EMU8000_DATA0, data)	//Unknown register
#define R_00A0_W(ch, data)	emu8000_reg_write_dword(5, ch, EMU8000_DATA0, data)	//Unknown register

#define CPF_R(ch)			emu8000_reg_read_dword(0, ch, EMU8000_DATA0)
#define PTRX_R(ch)			emu8000_reg_read_dword(1, ch, EMU8000_DATA0)
#define CVCF_R(ch)			emu8000_reg_read_dword(2, ch, EMU8000_DATA0)
#define VTFT_R(ch)			emu8000_reg_read_dword(3, ch, EMU8000_DATA0)
#define PSST_R(ch)			emu8000_reg_read_dword(6, ch, EMU8000_DATA0)
#define CSL_R(ch)			emu8000_reg_read_dword(7, ch, EMU8000_DATA0)
#define CCCA_R(ch)			emu8000_reg_read_dword(0, ch, EMU8000_DATA1)
#define HWCF4_R				emu8000_reg_read_dword(1, 9, EMU8000_DATA1)
#define HWCF5_R				emu8000_reg_read_dword(1, 10, EMU8000_DATA1)
#define HWCF6_R				emu8000_reg_read_dword(1, 13, EMU8000_DATA1)
#define HWCF7_R				emu8000_reg_read_dword(1, 14, EMU8000_DATA1)	//HWCF7 register
#define SMALR_R				emu8000_reg_read_dword(1, 20, EMU8000_DATA1)	//is not documented
#define SMARR_R				emu8000_reg_read_dword(1, 21, EMU8000_DATA1)
#define SMALW_R				emu8000_reg_read_dword(1, 22, EMU8000_DATA1)
#define SMARW_R				emu8000_reg_read_dword(1, 23, EMU8000_DATA1)
#define SMLD_R				emu8000_reg_read_word(1, 26, EMU8000_DATA1)
#define SMRD_R				emu8000_reg_read_word(1, 26, EMU8000_DATA2)
#define WC_R				emu8000_reg_read_word(1, 27, EMU8000_DATA2)
#define HWCF1_R				emu8000_reg_read_word(1, 29, EMU8000_DATA1)
#define HWCF2_R				emu8000_reg_read_word(1, 30, EMU8000_DATA1)
#define HWCF3_R				emu8000_reg_read_word(1, 31, EMU8000_DATA1)
#define INIT1_R(ch)			emu8000_reg_read_word(2, ch, EMU8000_DATA1)
#define INIT2_R(ch)			emu8000_reg_read_word(2, ch, EMU8000_DATA2)
#define INIT3_R(ch)			emu8000_reg_read_word(3, ch, EMU8000_DATA1)
#define INIT4_R(ch)			emu8000_reg_read_word(3, ch, EMU8000_DATA2)
#define ENVVOL_R(ch)		emu8000_reg_read_word(4, ch, EMU8000_DATA1)
#define DCYSUSV_R(ch)		emu8000_reg_read_word(5, ch, EMU8000_DATA1)
#define ENVVAL_R(ch)		emu8000_reg_read_word(6, ch, EMU8000_DATA1)
#define DCYSUS_R(ch)		emu8000_reg_read_word(7, ch, EMU8000_DATA1)
#define ATKHLDV_R(ch)		emu8000_reg_read_word(4, ch, EMU8000_DATA2)
#define LFO1VAL_R(ch)		emu8000_reg_read_word(5, ch, EMU8000_DATA2)
#define ATKHLD_R(ch)		emu8000_reg_read_word(6, ch, EMU8000_DATA2)
#define LFO2VAL_R(ch)		emu8000_reg_read_word(7, ch, EMU8000_DATA2)
#define IP_R(ch)			emu8000_reg_read_word(0, ch, EMU8000_DATA3)
#define IFATN_R(ch)			emu8000_reg_read_word(1, ch, EMU8000_DATA3)
#define PEFE_R(ch)			emu8000_reg_read_word(2, ch, EMU8000_DATA3)
#define FMMOD_R(ch)			emu8000_reg_read_word(3, ch, EMU8000_DATA3)
#define TREMFRQ_R(ch)		emu8000_reg_read_word(4, ch, EMU8000_DATA3)
#define FM2FRQ2_R(ch)		emu8000_reg_read_word(5, ch, EMU8000_DATA3)
#define R_0080_R(ch)		emu8000_reg_read_dword(4, ch, EMU8000_DATA0)	//Unknown register
#define R_00A0_R(ch)		emu8000_reg_read_dword(5, ch, EMU8000_DATA0)	//Unknown register

#endif /* EMU8000_REG_H_ */
