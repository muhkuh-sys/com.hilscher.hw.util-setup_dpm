/*---------------------------------------------------------------------------
  Author : Christoph Thelen

           Hilscher GmbH, Copyright (c) 2014, All Rights Reserved

           Redistribution or unauthorized use without expressed written
           agreement from the Hilscher GmbH is forbidden
---------------------------------------------------------------------------*/


#ifndef __BOOT_SPI_FLASH_H__
#define __BOOT_SPI_FLASH_H__

#include "boot_common.h"
#include "boot_spi.h"

/*-------------------------------------*/

#define SPI_FLASH_FLAGS_Forbid_Autodetection_of_Dummy_and_Idle  0x01U
#define SPI_FLASH_FLAGS_4_bit_address                           0x02U
#define SPI_FLASH_FLAGS_Atmel_address                           0x04U

typedef struct STRUCT_BOOT_SPI_FLASH_CONFIGURATION
{
	BOOT_SPI_CONFIGURATION_T tSpiCfg;
	unsigned int   uiMaximumSpeedInFifoMode_kHz;
	unsigned int   uiMaximumSpeedInRomMode_kHz;
	unsigned short usAtmelDataflashPageSize;
	unsigned char  ucFlags;
	unsigned char  ucReadCommand;
	unsigned char  ucReadCommandBusWidths;
	unsigned char  ucReadCommandDummyBytes;
	unsigned char  ucReadCommandIdleCycles;
} BOOT_SPI_FLASH_CONFIGURATION_T;


typedef struct STRUCT_BOOT_SQIROM_CONFIGURATION
{
	unsigned long ulSqiRomCfg;
	unsigned char aucSeqActivate[1+63];
	unsigned char aucSeqDeactivate[1+15];
} BOOT_SQIROM_CONFIGURATION_T;


/*-------------------------------------*/

BOOTING_T boot_spi_flash(PFN_SPI_INIT pfnInit, unsigned int uiSpiUnit, unsigned int uiSpiDevice, unsigned int uiChipSelect);

/*-------------------------------------*/


#endif	/* __BOOT_SPI_FLASH_H__ */

