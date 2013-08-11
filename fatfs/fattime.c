/*
 * fattime.c
 *
 *  Created on: 15-07-2013
 *      Author: Polprzewodnikowy
 */

#include "integer.h"
#include "fattime.h"

DWORD get_fattime(void)
{
	return
	(((DWORD)2012 - 1980) << 25)
	| ((DWORD)04 << 21)
	| ((DWORD)01 << 16)
	| (WORD)(12 << 11)
	| (WORD)(00 << 5)
	| (WORD)(00 >> 1);
}
