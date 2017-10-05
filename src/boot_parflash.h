/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __BOOT_PARFLASH_H__
#define __BOOT_PARFLASH_H__

#include "boot_common.h"
#include "memory.h"


typedef struct PL353_NOR_TIMINGS_STUCT
{
	unsigned short usT_RC;
	unsigned short usT_WC;
	unsigned char  ucT_CEOE;
	unsigned char  ucT_WP;
	unsigned char  ucT_PC;
	unsigned char  ucT_TR;
	unsigned char  ucWeTime;
} PL353_NOR_TIMINGS_T;



typedef struct PL353_NOR_SETTINGS_STUCT
{
	MEMORY_WIDTH_T tBusWidth;

	unsigned char ucBurstAlign;
	unsigned char ucBlsTime;
	unsigned char ucAdV;
	unsigned char ucBAA;
	unsigned char ucWrBl;
	unsigned char ucWrSync;
	unsigned char ucRdBl;
	unsigned char ucRdSync;

	PL353_NOR_TIMINGS_T tTimingsNs;
	unsigned short ausPortControl08[49];
	unsigned short ausPortControl16[49];
} PL353_NOR_SETTINGS_T;


BOOTING_T boot_pfl_netx(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect);

BOOTING_T boot_pl353_nor(unsigned int uiChipSelect);


#endif  /* __BOOT_PARFLASH_H__ */

