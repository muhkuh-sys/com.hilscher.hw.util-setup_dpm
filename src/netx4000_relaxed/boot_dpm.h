/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/

#include "boot_common.h"
#include "netx_io_areas.h"

#ifndef __BOOT_DPM_H__
#define __BOOT_DPM_H__


/*-----------------------------------*/


#define DPM_NETX_TO_HOST_BUFFERSIZE	0x0200
#define DPM_HOST_TO_NETX_BUFFERSIZE	0x0400


typedef enum DPM_TRANSPORT_TYPE_ENUM
{
	DPM_TRANSPORT_TYPE_Parallel  = 0,
	DPM_TRANSPORT_TYPE_Serial    = 1
} DPM_TRANSPORT_TYPE_T;


typedef enum
{
	DPM_ConnectionState_Idle		= 0,
	DPM_ConnectionState_Connected		= 1
} DPM_ConnectionState_t;



typedef struct PCIE_BOOT_OPTIONS_STRUCT
{
	unsigned long ulPowerUpTimeoutTicks;
	unsigned long ulResetDelayTicks;
	unsigned long ulVidPid;
	unsigned long ulRevisionClassCode;
	unsigned short usSysctrlPcieCfg;
	unsigned char ucAcpCachePolicyPcie;
} PCIE_BOOT_OPTIONS_T;



typedef struct DPM_INFORMATION_BLOCK_STRUCT
{
	volatile unsigned long ulDpmBootId;
	volatile unsigned long ulDpmByteSize;
	volatile unsigned long aulChipId[4];
	volatile unsigned long aulAsrId[10];
	char acId[56];
	volatile unsigned long ulNetxToHostDataSize;
	volatile unsigned long ulHostToNetxDataSize;
} DPM_INFORMATION_BLOCK_T;



typedef struct DPM_HANDSHAKE_BLOCK_STRUCT
{
	volatile unsigned long aulHandshakeReg[16];
	volatile unsigned long aulReserved_90[16];
} DPM_HANDSHAKE_BLOCK_T;



typedef struct DPM_MAILBOX_BLOCK_STRUCT
{
	volatile unsigned char aucNetxToHostData[DPM_NETX_TO_HOST_BUFFERSIZE];
	volatile unsigned char aucHostToNetxData[DPM_HOST_TO_NETX_BUFFERSIZE];
} DPM_MAILBOX_BLOCK_T;



typedef struct DPM_BLOCKS_STRUCTURE
{
	DPM_INFORMATION_BLOCK_T tInformationBlock;
	DPM_HANDSHAKE_BLOCK_T tHandshakeBlock;
	DPM_MAILBOX_BLOCK_T tMailboxBlock;
} DPM_BLOCKS_T;



typedef struct BOOT_PCIE_AREA_STRUCTURE
{
	NX4000_IDPM_AREA_T tPseudoConfigurationRegisters;
	DPM_BLOCKS_T tDpmBlocks;
	const unsigned long aulBitflipArea[0xf800/sizeof(unsigned long)];
} BOOT_PCIE_AREA_T;



struct DPM_HANDLE_STRUCT;

typedef void (*PFN_DPM_SET_STATE_T)(struct DPM_HANDLE_STRUCT *ptHandle);
typedef unsigned long (*PFN_DPM_GET_STATE_DIFFERENCES_T)(void);


typedef enum DPM_CONNECTION_ENUM
{
	DPM_CONNECTION_DPM  = 0,
	DPM_CONNECTION_IDPM = 1,
	DPM_CONNECTION_PCIE = 2
} DPM_CONNECTION_T;

typedef struct DPM_HANDLE_STRUCT
{
	DPM_BLOCKS_T *ptDpmBlocks;
	const unsigned long *pulBitflipArea;

	PFN_DPM_SET_STATE_T pfnSetState;
	PFN_DPM_GET_STATE_DIFFERENCES_T pfnGetStateDifferences;
	volatile unsigned long *pulHandshakeRegister;

	unsigned int uiNetxToHostBufferIdx;
	unsigned int uiHostToNetxBufferIdx;

	unsigned long ulNetxState;
	unsigned long ulNetxStatusCode;

	DPM_CONNECTION_T tConnection;
} DPM_HANDLE_T;


/*-------------------------------------------------------------------------*/


BOOTING_T boot_dpm(DPM_TRANSPORT_TYPE_T tDpmTransportType);
BOOTING_T boot_pcie(void);


/*-----------------------------------*/


#endif  /* __BOOT_DPM_H__ */
