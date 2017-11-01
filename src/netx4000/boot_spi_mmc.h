/*---------------------------------------------------------------------------
  Author : Christoph Thelen

           Hilscher GmbH, Copyright (c) 2006, All Rights Reserved

           Redistribution or unauthorized use without expressed written 
           agreement from the Hilscher GmbH is forbidden
---------------------------------------------------------------------------*/


#ifndef __BOOT_SPI_MMC_H__
#define __BOOT_SPI_MMC_H__

#include "boot_common.h"
#include "boot_spi.h"


/*
 * MMC SPI Commands
 */
typedef enum ENUM_MMC_COMMAND
{
	MMC_Cmd00_GO_IDLE_STATE		= 0,	/* resets the mmc */
	MMC_Cmd01_SEND_OP_COND		= 1,	/* activate the card's init process */
	MMC_Cmd09_SEND_CSD		= 9,	/* ask the card to send it's Card Specific Data */
	MMC_Cmd10_SEND_CID		= 10,	/* ask the card to send it's Card IDentification */
	MMC_Cmd12_STOP_TRANSMISSION	= 12,	/* stops a multiple block transmission */
	MMC_Cmd13_SEND_STATUS		= 13,	/* ask the card to send it's status register */
	MMC_Cmd16_SET_BLOCKLEN		= 16,	/* select blocklength (in bytes) for all block commands */
	MMC_Cmd17_READ_SINGLE_BLOCK	= 17,	/* reads a block of the size selected by Cmd16 */
	MMC_Cmd18_READ_MULTIPLE_BLOCK	= 18,	/* reads multiple blocks of the size selected by Cmd16 */
	MMC_Cmd24_WRITE_BLOCK		= 24,	/* writes a block of the size selected by Cmd16 */
	MMC_Cmd27_PROGRAM_CSD		= 27,	/* programming of the programmable bits of the CSD */
	MMC_Cmd28_SET_WRITE_PROT	= 28,	/* if the card has write protect features, this cmd sets
						   the write protection bit of the addressed group */
	MMC_Cmd29_CLR_WRITE_PROT	= 29,	/* if the card has write protect features, this cmd clears
						   the write protection bit of the addressed group */
	MMC_Cmd30_SEND_WRITE_PROT	= 30,	/* if the card has write protect features, this cmd asks
						   the card to send the status of the write protection bits */
	MMC_Cmd32_TAG_SECTOR_START	= 32,	/* set the first sector of the erase group */
	MMC_Cmd33_TAG_SECTOR_END	= 33,	/* set the last sector of the erase group or a single sector */
	MMC_Cmd34_UNTAG_SECTOR		= 34,	/* removes a previously selected sector from the erase group */
	MMC_Cmd35_TAG_ERASE_GROUP_START	= 35,	/* set the address of the first erase group */
	MMC_Cmd36_TAG_ERASE_GROUP_END	= 36,	/* set the address of the last erase group */
	MMC_Cmd37_UNTAG_ERASE_GROUP	= 37,	/* removes one previously selected erase group from the selection */
	MMC_Cmd38_ERASE			= 38,	/* erase all selected sectors */
	MMC_Cmd59_CRC_ON_OFF		= 59	/* turns crc protection on or off depending on the argument */
} MMC_COMMAND_T;

/*
 * MMC Response Format 1
 */
#define MMC_R1_Idle             0x01
#define MMC_R1_EraseReset       0x02
#define MMC_R1_IllegalCommand   0x04
#define MMC_R1_CRCError         0x08
#define MMC_R1_EraseSeqError    0x10
#define MMC_R1_AddressError     0x20
#define MMC_R1_ParameterError   0x40

/*
 * Spi config
 */
/* spi slave device of the MMC slot */
#define BootSPIMMC_Slave                2U

/* Idle byte for MMC operation */
#define BootSPIMMC_IdleByte             0xffU

/* assumed sector size for mmc */
#define BootSPIMMC_sectorSize           0x0200U
/* shifts needed to translate sector<->address */
#define BootSPIMMC_sectorBitShift       9U
/* bitmask for byteaccess in the sector */
#define BootSPIMMC_sectorMask           0x01ffU
/* address of the master boot record */
#define BootSPIMMC_mbrAddress           0U
/* size of one directory entry in bytes */
#define BootSPIMMC_dirEntrySize         0x20U
/* shifts needed to replace multiplication by dirEntrySize */
#define BootSPIMMC_dirEntryShift        5U
/* number of directory entries per sector */
#define BootSPIMMC_dirEntriesPerSector  16U
/* size of the csd structure in bytes */
#define BootSPIMMC_csdSize		16U

/*-------------------------------------*/

typedef struct STRUCT_BOOT_SPI_MMC_CONFIGURATION
{
	BOOT_SPI_CONFIGURATION_T tSpiCfg;
	unsigned long ulSpeedLimitKHz;
	unsigned long ulGoIdleRetryDelayMs;
	unsigned long ulGetStatusRetries;
	unsigned char atFileName[12+1];
	unsigned char ucInitialIdles;
} BOOT_SPI_MMC_CONFIGURATION_T;

/*-------------------------------------*/

BOOTING_T boot_spi_mmc(PFN_SPI_INIT pfnInit, unsigned int uiSpiDevice, unsigned int uiChipSelect);

/*-------------------------------------*/

#endif	/* __BOOT_SPI_MMC_H__ */

