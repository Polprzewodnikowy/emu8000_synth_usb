//***********************************************************************
// Plik: keyb.h 
//
// Zaawansowana obsługa przycisków i klawiatur 
// Wersja:    1.0
// Licencja:  GPL v2
// Autor:     Deucalion  
// Email:     deucalion#wp.pl
// Szczegóły: http://mikrokontrolery.blogspot.com/2011/04/jezyk-c-biblioteka-obsluga-klawiatury.html
//
//***********************************************************************

#ifndef _KEYB_H_
#define _KEYB_H_

#include <inttypes.h>
#include "config.h"

#define KEY1_bb		(USER_KEY_rbb<<0)
#define KEY2_bb		0

#define KEY1		(1<<0)
#define KEY2		(1<<1)
#define ANYKEY		(KEY1 | KEY2)
#define KEY_MASK	(KEY1 | KEY2)

#define KBD_LOCK	1
#define KBD_NOLOCK	0

#define KBD_DEFAULT_ART	((void *)0)

void ClrKeyb(int lock);
void KeybProc(void);
unsigned int GetKeys(void);
unsigned int KeysTime(void);
unsigned int IsKeyPressed(unsigned int mask);
unsigned int IsKey(unsigned int mask);
void KeybLock(void);
void KeybSetAutoRepeatTimes(unsigned short *AutoRepeatTab);

#endif
