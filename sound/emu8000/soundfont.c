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
int used_ram;

#define NEW(type, nums)	(type *)malloc(sizeof(type) * (nums))

#define READID(var, fd)		READDW(var, fd)
#define READB(var, fd)		f_read(fd, var, 1, &bytes)
#define READW(var, fd)		f_read(fd, var, 2, &bytes)
#define READDW(var, fd)		f_read(fd, var, 4, &bytes)
#define READCHUNK(var, fd)	f_read(fd, var, 8, &bytes)
#define READSTR(var,fd)		f_read(fd, var, 20, &bytes)

static int getchunk(const char *id);
static void process_chunk(int id, int s, SFInfo *sf, FIL *fd);
static void load_preset_header(int size, SFInfo *sf, FIL *fd);
static void load_inst_header(int size, SFInfo *sf, FIL *fd);
static void load_bag(int size, FIL *fd, int *totalp, uint16_t **bufp);
static void load_gen(int size, FIL *fd, int *totalp, genrec_t **bufp);
static void load_sample_info(int size, SFInfo *sf, FIL *fd);

enum {
	/* level 0 */
	UNKN_ID, RIFF_ID, LIST_ID,
	/* level 1 */
	INFO_ID, SDTA_ID, PDTA_ID,
	/* info stuff */
	IFIL_ID, ISNG_ID, IROM_ID, INAM_ID, IVER_ID, IPRD_ID, ICOP_ID,
	/* sample data stuff */
	SNAM_ID, SMPL_ID,
	/* preset stuff */
	PHDR_ID, PBAG_ID, PMOD_ID, PGEN_ID,
	/* inst stuff */
	INST_ID, IBAG_ID, IMOD_ID, IGEN_ID,
	/* sample header */
	SHDR_ID
};

int load_soundfont(SFInfo *sf, char *path)
{
	chunk_t chunk, subchunk;
	FIL fd;
	used_ram = 0;

	if(f_open(&fd, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
		return -1;

	f_lseek(&fd, f_tell(&fd) + 12); //RIFF, size, SFBK

	while(!f_eof(&fd))
	{
		READID(chunk.id, &fd);
		switch(getchunk(chunk.id))
		{
		case LIST_ID:
			READDW(&chunk.size, &fd);
			READID(subchunk.id, &fd);
			process_chunk(getchunk(subchunk.id), chunk.size - 4, sf, &fd);
			break;
		}
	}

	sf->used_ram = used_ram + 40; //10 * 4-byte ints

	f_close(&fd);

	return 0;
}

void free_soundfont(SFInfo *sf)
{
	free(sf->presethdr);
	free(sf->sampleinfo);
	free(sf->insthdr);
	free(sf->presetbag);
	free(sf->instbag);
	free(sf->presetgen);
	free(sf->instgen);
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
	};

	unsigned int i;

	for(i = 0; i < sizeof(idlist) / sizeof(idlist[0]); i++)
	{
		if(strncmp(id, idlist[i].str, 4) == 0)
			return idlist[i].id;
	}

	return UNKN_ID;
}

static void load_preset_header(int size, SFInfo *sf, FIL *fd)
{
	int i;
	sf->nrpresets = size / 38;
	used_ram += sf->nrpresets * sizeof(presethdr_t);
	sf->presethdr = NEW(presethdr_t, sf->nrpresets);
	for(i = 0; i < sf->nrpresets; i++)
	{
		READSTR(sf->presethdr[i].name, fd); //f_lseek(fd, f_tell(fd) + 20); //Preset name
		READW(&sf->presethdr[i].preset, fd);
		READW(&sf->presethdr[i].bank, fd);
		READW(&sf->presethdr[i].bagNdx, fd);
		f_lseek(fd, f_tell(fd) + 12); //lib, genre, morph
	}
}

static void load_inst_header(int size, SFInfo *sf, FIL *fd)
{
	int i;
	sf->nrinsts = size / 22;
	used_ram += sf->nrinsts * sizeof(insthdr_t);
	sf->insthdr = NEW(insthdr_t, sf->nrinsts);
	for(i = 0; i < sf->nrinsts; i++)
	{
		f_lseek(fd, f_tell(fd) + 20); //Instrument name
		READW(&sf->insthdr[i].bagNdx, fd);
	}
}

static void load_bag(int size, FIL *fd, int *totalp, uint16_t **bufp)
{
	uint16_t *buf;
	int i;
	size /= 4;
	used_ram += size * sizeof(uint16_t);
	buf = NEW(uint16_t, size);
	for(i = 0; i < size; i++)
	{
		READW(&buf[i], fd);
		f_lseek(fd, f_tell(fd) + 2); //mod
	}
	*totalp = size;
	*bufp = buf;
}

static void load_gen(int size, FIL *fd, int *totalp, genrec_t **bufp)
{
	genrec_t *buf;
	int i;
	size /= 4;
	used_ram += size * sizeof(genrec_t);
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

	sf->nrinfos = size / 46;
	sf->sampleinfo = NEW(sampleinfo_t, sf->nrinfos);
	used_ram += sf->nrinfos * sizeof(sampleinfo_t);
	for(i = 0; i < sf->nrinfos; i++)
	{
		f_lseek(fd, f_tell(fd) + 20); //Sample names
		READDW(&sf->sampleinfo[i].startsample, fd);
		READDW(&sf->sampleinfo[i].endsample, fd);
		READDW(&sf->sampleinfo[i].startloop, fd);
		READDW(&sf->sampleinfo[i].endloop, fd);

		READDW(&sf->sampleinfo[i].samplerate, fd);
		READB(&sf->sampleinfo[i].originalPitch, fd);
		READB(&sf->sampleinfo[i].pitchCorrection, fd);
		f_lseek(fd, f_tell(fd) + 2); //Sample link
		READW(&sf->sampleinfo[i].sampletype, fd);
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
				f_lseek(fd, f_tell(fd) + 4); //Version, minor version
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
				f_lseek(fd, f_tell(fd) + subchunk.size); //Sample names
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
			case PMOD_ID: //Ignored
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
			case IMOD_ID: //Ingored
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

