/*
 * soundfont.c
 *
 *  Created on: 17-07-2013
 *      Author: Polprzewodnikowy
 *      Source: https://github.com/alsaplayer/alsaplayer/tree/master/attic/midi
 */

#include <stdlib.h>
#include <string.h>
#include "sound/emu8000/soundfont.h"

UINT bytes;

#define NEW(type, nums)	(type *)malloc(sizeof(type) * (nums))

static int READCHUNK(chunk_t *vp, FIL *fd)
{
	if(f_read(fd, vp, 8, &bytes) != FR_OK)
		return -1;
	return 1;
}

static int READDW(uint32_t *vp, FIL *fd)
{
	if(f_read(fd, vp, 4, &bytes) != FR_OK)
		return -1;
	return 1;
}

static int READW(uint16_t *vp, FIL *fd)
{
	if(f_read(fd, vp, 2, &bytes) != FR_OK)
		return -1;
	return 1;
}

#define READSTR(var, fd)	f_read(fd, var, 20, &bytes)

#define READID(var, fd)	f_read(fd, var, 4, &bytes)
#define READB(var, fd)	f_read(fd, var, 1, &bytes)
#define SKIPB(fd)		{uint8_t dummy[1]; f_read(fd, dummy, 1, &bytes);}
#define SKIPW(fd)		{uint8_t dummy[2]; f_read(fd, dummy, 2, &bytes);}
#define SKIPDW(fd)		{uint8_t dummy[4]; f_read(fd, dummy, 4, &bytes);}

static int getchunk(const char *id);
static void process_chunk(int id, int s, SFInfo *sf, FIL *fd);
static void load_sample_names(int size, SFInfo *sf, FIL *fd);
static void load_preset_header(int size, SFInfo *sf, FIL *fd);
static void load_inst_header(int size, SFInfo *sf, FIL *fd);
static void load_bag(int size, FIL *fd, int *totalp, unsigned short **bufp);
static void load_gen(int size, FIL *fd, int *totalp, genrec_t **bufp);
static void load_sample_info(int size, SFInfo *sf, FIL *fd);

enum {
	/* level 0 */
	UNKN_ID, RIFF_ID, LIST_ID,
	/* level 1 */
	INFO_ID, SDTA_ID, PDTA_ID,
	/* info stuff */
	IFIL_ID, ISNG_ID, IROM_ID, INAM_ID, IVER_ID, IPRD_ID, ICOP_ID,
#ifdef tplussbk
	ICRD_ID, IENG_ID, ISFT_ID, ICMT_ID,
#endif
	/* sample data stuff */
	SNAM_ID, SMPL_ID,
	/* preset stuff */
	PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,
	/* inst stuff */
	INST_ID, IBAG_ID, IMOD_ID, IGEN_ID,
	/* sample header */
	SHDR_ID
};

void load_soundfont(FIL *fd, SFInfo *sf)
{
	chunk_t chunk, subchunk;

	READID(sf->sfhdr.riff, fd);
	READDW(&sf->sfhdr.size, fd);
	READID(sf->sfhdr.sfbk, fd);

	sf->inrom = 1;
	while(!f_eof(fd))
	{
		READID(chunk.id, fd);
		switch(getchunk(chunk.id))
		{
		case LIST_ID:
			READDW(&chunk.size, fd);
			READID(subchunk.id, fd);
			process_chunk(getchunk(subchunk.id), chunk.size - 4, sf, fd);
			break;
		}
	}
}

int open_soundfont(SFInfo *sf, char *path)
{
	FIL sbk_file;
	if(f_open(&sbk_file, path, FA_OPEN_EXISTING | FA_READ) == FR_OK)
	{
		load_soundfont(&sbk_file, sf);
		f_close(&sbk_file);
	}else{
		return -1;
	}
	return 0;
}

void free_soundfont(SFInfo *sf)
{
	free(sf->samplenames);
	free(sf->presethdr);
	free(sf->sampleinfo);
	free(sf->insthdr);
	free(sf->presetbag);
	free(sf->instbag);
	free(sf->presetgen);
	free(sf->instgen);
	if(sf->sfname)
		free(sf->sfname);
	memset(sf, 0, sizeof(*sf));
}

static int getchunk(const char *id)
{
	static struct idstring
	{
		const char *str;
		int id;
	} idlist[] = {
		{"LIST", LIST_ID},
#ifdef tplussbk
		{"sfbk", SFBK_ID},
#endif
		{"INFO", INFO_ID},
		{"sdta", SDTA_ID},
		{"snam", SNAM_ID},
		{"smpl", SMPL_ID},
		{"pdta", PDTA_ID},
		{"phdr", PHDR_ID},
		{"pbag", PBAG_ID},
		{"pmod", PMOD_ID},
		{"pgen", PGEN_ID},
		{"inst", INST_ID},
		{"ibag", IBAG_ID},
		{"imod", IMOD_ID},
		{"igen", IGEN_ID},
		{"shdr", SHDR_ID},
		{"ifil", IFIL_ID},
		{"isng", ISNG_ID},
		{"irom", IROM_ID},
		{"iver", IVER_ID},
		{"INAM", INAM_ID},
		{"IPRD", IPRD_ID},
		{"ICOP", ICOP_ID},
#ifdef tplussbk
		{"ICRD", ICRD_ID},
		{"IENG", IENG_ID},
		{"ISFT", ISFT_ID},
		{"ICMT", ICMT_ID},
#endif
	};

	unsigned int i;

	for(i = 0; i < sizeof(idlist) / sizeof(idlist[0]); i++)
	{
		if(strncmp(id, idlist[i].str, 4) == 0)
			return idlist[i].id;
	}

	return UNKN_ID;
}

static void load_sample_names(int size, SFInfo *sf, FIL *fd)
{
	int i;
	sf->nrsamples = size / 20;
	sf->samplenames = NEW(samplenames_t, sf->nrsamples);
	for(i = 0; i < sf->nrsamples; i++)
	{
		READSTR(sf->samplenames[i].name, fd);
		sf->samplenames[i].name[20] = 0;
	}
}

static void load_preset_header(int size, SFInfo *sf, FIL *fd)
{
	int i;
	sf->nrpresets = size / 38;
	sf->presethdr = NEW(presethdr_t, sf->nrpresets);
	for(i = 0; i < sf->nrpresets; i++)
	{
		READSTR(sf->presethdr[i].name, fd);
		READW(&sf->presethdr[i].preset, fd);
		sf->presethdr[i].sub_preset = sf->presethdr[i].preset;
		READW(&sf->presethdr[i].bank, fd);
		sf->presethdr[i].sub_bank = sf->presethdr[i].bank;
		READW(&sf->presethdr[i].bagNdx, fd);
		SKIPDW(fd);	/* lib */
		SKIPDW(fd);	/* genre */
		SKIPDW(fd);	/* morph */
	}
}

static void load_inst_header(int size, SFInfo *sf, FIL *fd)
{
	int i;
	sf->nrinsts = size / 22;
	sf->insthdr = NEW(insthdr_t, sf->nrinsts);
	for(i = 0; i < sf->nrinsts; i++)
	{
		READSTR(sf->insthdr[i].name, fd);
		READW(&sf->insthdr[i].bagNdx, fd);
	}
}

static void load_bag(int size, FIL *fd, int *totalp, unsigned short **bufp)
{
	unsigned short *buf;
	int i;
	size /= 4;
	buf = NEW(unsigned short, size);
	for(i = 0; i < size; i++)
	{
		READW(&buf[i], fd);
		SKIPW(fd);	/* mod */
	}
	*totalp = size;
	*bufp = buf;
}

static void load_gen(int size, FIL *fd, int *totalp, genrec_t **bufp)
{
	genrec_t *buf;
	int i;
	size /= 4;
	buf = NEW(genrec_t, size);
	for(i = 0; i < size; i++)
	{
		READW((uint16_t *)&buf[i].oper, fd);
		READW(&buf[i].amount, fd);
	}
	*totalp = size;
	*bufp = buf;
}

static void load_sample_info(int size, SFInfo *sf, FIL *fd)
{
	int i;
	if(sf->version > 1)
	{
		sf->nrinfos = size / 46;
		sf->nrsamples = sf->nrinfos;
		sf->sampleinfo = NEW(sampleinfo_t, sf->nrinfos);
		sf->samplenames = NEW(samplenames_t, sf->nrsamples);
	}else{
		sf->nrinfos = size / 16;
		sf->sampleinfo = NEW(sampleinfo_t, sf->nrinfos);
	}

	for(i = 0; i < sf->nrinfos; i++)
	{
		if(sf->version > 1)
			READSTR(sf->samplenames[i].name, fd);
		READDW(&sf->sampleinfo[i].startsample, fd);
		READDW(&sf->sampleinfo[i].endsample, fd);
		READDW(&sf->sampleinfo[i].startloop, fd);
		READDW(&sf->sampleinfo[i].endloop, fd);
		if(sf->version > 1)
		{
			READDW(&sf->sampleinfo[i].samplerate, fd);
			READB(&sf->sampleinfo[i].originalPitch, fd);
			READB(&sf->sampleinfo[i].pitchCorrection, fd);
			READW(&sf->sampleinfo[i].samplelink, fd);
			READW(&sf->sampleinfo[i].sampletype, fd);
		}else{
			if(sf->sampleinfo[i].startsample == 0)
				sf->inrom = 0;
			sf->sampleinfo[i].startloop++;
			sf->sampleinfo[i].endloop += 2;
			sf->sampleinfo[i].samplerate = 44100;
			sf->sampleinfo[i].originalPitch = 60;
			sf->sampleinfo[i].pitchCorrection = 0;
			sf->sampleinfo[i].samplelink = 0;
			if(sf->inrom)
				sf->sampleinfo[i].sampletype = 0x8001;
			else
				sf->sampleinfo[i].sampletype = 1;
		}
	}
}

static void process_chunk(int id, int s, SFInfo *sf, FIL *fd)
{
	int cid;
	chunk_t subchunk;
	s = s;

	switch(id)
	{
	case INFO_ID:
		READCHUNK(&subchunk, fd);
		while((cid = getchunk(subchunk.id)) != LIST_ID)
		{
			switch(cid)
			{
			case IFIL_ID:
				READW(&sf->version, fd);
				READW(&sf->minorversion, fd);
				break;
			case INAM_ID:
				sf->sfname = (char *)malloc(subchunk.size);
				if(sf->sfname != NULL)
					f_read(fd, sf->sfname, subchunk.size, &bytes);
				break;
			default:
				f_lseek(fd, f_tell(fd) + subchunk.size);
				break;
			}
			READCHUNK(&subchunk, fd);
			if(f_eof(fd))
				return;
		}
		f_lseek(fd, f_tell(fd) - 8);
		break;
	case SDTA_ID:
		READCHUNK(&subchunk, fd);
		while((cid = getchunk(subchunk.id)) != LIST_ID)
		{
			switch(cid)
			{
			case SNAM_ID:
				if(sf->version > 1)
				{
					f_lseek(fd, f_tell(fd) + subchunk.size);
				}else{
					load_sample_names(subchunk.size, sf, fd);
				}
				break;
			case SMPL_ID:
				sf->samplepos = f_tell(fd);
				sf->samplesize = subchunk.size;
				f_lseek(fd, f_tell(fd) + subchunk.size);
			}
			READCHUNK(&subchunk, fd);
			if(f_eof(fd))
				return;
		}
		f_lseek(fd, f_tell(fd) - 8);
		break;
	case PDTA_ID:
		READCHUNK(&subchunk, fd);
		while((cid = getchunk(subchunk.id)) != LIST_ID)
		{
			switch(cid)
			{
			case PHDR_ID:
				load_preset_header(subchunk.size, sf, fd);
				break;
			case PBAG_ID:
				load_bag(subchunk.size, fd, &sf->nrpbags, &sf->presetbag);
				break;
			case PMOD_ID: /* ignored */
				f_lseek(fd, f_tell(fd) + subchunk.size);
				break;
			case PGEN_ID:
				load_gen(subchunk.size, fd, &sf->nrpgens, &sf->presetgen);
				break;
			case INST_ID:
				load_inst_header(subchunk.size, sf, fd);
				break;
			case IBAG_ID:
				load_bag(subchunk.size, fd, &sf->nribags, &sf->instbag);
				break;
			case IMOD_ID: /* ingored */
				f_lseek(fd, f_tell(fd) + subchunk.size);
				break;
			case IGEN_ID:
				load_gen(subchunk.size, fd, &sf->nrigens, &sf->instgen);
				break;
			case SHDR_ID:
				load_sample_info(subchunk.size, sf, fd);
				break;
			default:
				f_lseek(fd, f_tell(fd) + subchunk.size);
				break;
			}
			READCHUNK(&subchunk, fd);
			if(f_eof(fd))
				return;
		}
		f_lseek(fd, f_tell(fd) - 8);
		break;
	}
}

