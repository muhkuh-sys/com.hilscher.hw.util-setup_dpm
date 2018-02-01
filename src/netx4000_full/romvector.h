/***************************************************************************
 *   Copyright (C) 2011 by Hilscher GmbH                                   *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __ROMVECTOR_H__
#define __ROMVECTOR_H__


/* NOTE: This headerfile is included in C and ASM sources. */


/* Chiptype defines. */
#define CHIPTYPE_NETX500              0x01
#define CHIPTYPE_NETX100              0x02
#define CHIPTYPE_NETX50               0x03
#define CHIPTYPE_NETX5                0x04
#define CHIPTYPE_NETX10               0x05
#define CHIPTYPE_NETX56               0x06
#define CHIPTYPE_NETX56B              0x07
#define CHIPTYPE_NETX4000_RELAXED     0x08
#define CHIPTYPE_NETX6                0x09
#define CHIPTYPE_NETX90_MPW           0x0a
#define CHIPTYPE_NETX4000_FULL_SMALL  0x0b


#define MSK_RomVector_Info_Version  0x00000fffU
#define SRT_RomVector_Info_Version  0
#define MSK_RomVector_Info_Chiptyp  0x000ff000U
#define SRT_RomVector_Info_Chiptyp  12
#define MSK_RomVector_Info_Step     0xfff00000U
#define SRT_RomVector_Info_Step     20


/* The typedefs can not be used in ASM sources. */
#if __ASSEMBLER__!=1
typedef struct
{
	/* This is the vector table at the start of the ROM. */
	unsigned long aulVectorTable[8];
	/* Version and chip type. */
	unsigned long ulInfo;
} ROMVECTOR_T;

extern ROMVECTOR_T tRomVector;
#endif


#endif  /* __ROMVECTOR_H__ */
