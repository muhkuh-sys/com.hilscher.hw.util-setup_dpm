/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include <stddef.h>

#include "boot_common.h"


#ifndef __BOOTBLOCK_OLDSTYLE_H__
#define __BOOTBLOCK_OLDSTYLE_H__


/* Magic Cookie */
#define BOOTBLOCK_OLDSTYLE_MAGIC        0xf3beaf00U
#define BOOTBLOCK_ALT_MAGIC             0xf3ad9e00U
#define BOOTBLOCK_OLDSTYLE_MAGIC_MASK   0xffffff00U

#define BOOTBLOCK_OLDSTYLE_MAGIC_PARAMETER_NOAUTO 0x0000001AU
#define BOOTBLOCK_OLDSTYLE_MAGIC_PARAMETER_MASK   0x000000FFU

/* 'NETX' signature in HEX format */
#define BOOTBLOCK_OLDSTYLE_SIGNATURE	0x5854454eU


/* application entry point is a "void blah(void)" function */
struct BOOTBLOCK_OLDSTYLE_Ttag;
typedef void(*pfnAppEntryPoint_t)(struct BOOTBLOCK_OLDSTYLE_Ttag *ptBootblock, BOOTOPTION_T bootOption);

/* Structure definition of the boot block. */
typedef struct BOOTBLOCK_OLDSTYLE_Ttag
{
	unsigned long ulMagic;

	union
	{
		unsigned long ulSRamCtrl;
		unsigned long ulSpiSpeed;
	} uBootCtrl;

	pfnAppEntryPoint_t pfnExecutionAddress;
	unsigned long ulApplicationChecksum;
	size_t sizApplicationDword;
	unsigned long *pulApplicationLoadAddress;
	unsigned long ulSignature;

	union
	{
		struct
		{
			unsigned long  ulGeneralCtrl;
			unsigned long  ulTimingCtrl;
			unsigned long  ulModeRegister;
			unsigned long aulReserved[2];
		} sSDRam;
		struct
		{
			unsigned long ulCtrl;
			unsigned long aulReserved[4];
		} sSRam;
		struct
		{
			unsigned long ulExpBusReg;
			unsigned long ulIoRegMode0;
			unsigned long ulIoRegMode1; 
			unsigned long ulIfConf1;
			unsigned long ulIfConf2;
		} sDpm;
	} uMemoryCtrl;

	unsigned long ulMiscAsicCtrl;
	unsigned long ulUserData;
	unsigned long ulSrcType;
	unsigned long ulBootChksm;
} BOOTBLOCK_OLDSTYLE_T;

typedef union
{
	BOOTBLOCK_OLDSTYLE_T s;
	unsigned long aul[16];
	unsigned char auc[64];
} BOOTBLOCK_OLDSTYLE_U_T;


unsigned long bootblock_oldstyle_bootblock_checksum(const unsigned long *pulBootblock);


#endif	/* __BOOTBLOCK_OLDSTYLE_H__ */
