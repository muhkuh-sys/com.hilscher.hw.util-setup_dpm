/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __BOOT_NAND_H__
#define __BOOT_NAND_H__

#include "boot_common.h"
#include "memory.h"



typedef struct PL353_TIMING_SET_STRUCT
{
	unsigned long ulT_RC;
	unsigned long ulT_WC;
	unsigned long ulT_REA;
	unsigned long ulT_WP;
	unsigned long ulT_CLR;
	unsigned long ulT_AR;
	unsigned long ulT_RR;
	unsigned long ulT_CEA;
} PL353_TIMING_SET_T;



typedef enum NAND_DEVICE_PARAMETERS_FLAG_ENUM
{
	NAND_DEVICE_PARAMETERS_FLAG_AutodetectReadySignaling   = 0x00000001U,
	NAND_DEVICE_PARAMETERS_FLAG_Use_ReadySignal0_Polling1  = 0x00000002U,  /* 0=use the ready line, 1=use polling with READ_STATUS command */
	NAND_DEVICE_PARAMETERS_FLAG_HasReadStatusEnhanced      = 0x00000004U
} NAND_DEVICE_PARAMETERS_FLAG_T;


typedef struct NAND_DEVICE_PARAMETERS_STRUCT
{
	unsigned long ulPageSize;
	unsigned long ulOobSize;
	unsigned long ulPageUsableSize;

	unsigned long ulPageMax;
	unsigned long ulBlockMax;
	unsigned long ulLunMax;

	unsigned long ulAddressCyclesColumn;
	unsigned long ulAddressCyclesRow;

	MEMORY_WIDTH_T tBusWidth;

	unsigned long ulFlags;

	unsigned long ulReadTimeoutMs;
	unsigned long ulT_RST;

	PL353_TIMING_SET_T tTimingsNs;

	unsigned short ausPortControlDetect[22];
	unsigned short ausPortControl08[22];
	unsigned short ausPortControl16[22];
} NAND_DEVICE_PARAMETERS_T;



typedef struct NAND_PREDEFINED_DEVICE_PARAMETERS_STRUCT
{
	unsigned long aulId[2];
	NAND_DEVICE_PARAMETERS_T tDeviceParameter;
} NAND_PREDEFINED_DEVICE_PARAMETERS_T;



BOOTING_T boot_pfl_nand(void);


#endif  /* __BOOT_NAND_H__ */

