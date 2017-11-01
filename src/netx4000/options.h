/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "console_driver_rap_uart.h"
#include "console_driver_uart.h"
#include "console_driver_usbdev.h"
#include "networking/boot_drv_eth.h"
#include "boot_common.h"
#include "boot_dpm.h"
#include "boot_nand.h"
#include "boot_parflash.h"
#include "boot_sdio.h"
#include "boot_spi_flash.h"
#include "boot_spi_mmc.h"
#include "console_io.h"
#include "i2c_eeprom.h"
#include "memory.h"


#define NUMBER_OF_MMIOS ARRAYSIZE(((NX4000_MMIO_CTRL_AREA_T*)NULL)->aulMmio_cfg)


typedef enum SQIROM_UNIT_OFFSET_ENUM
{
	SQIROM_UNIT_OFFSET_CURRENT       = 0,
	SQIROM_UNIT_OFFSET_SQIROM0_CS0   = 1,
	SQIROM_UNIT_OFFSET_SQIROM1_CS0   = 2
}  SQIROM_UNIT_OFFSET_T;


typedef struct STRUCT_SYSTEM_CONFIG
{
	unsigned long  ul_io_config;
	unsigned long  ul_io_config2;
	unsigned long  ul_clock_enable;
	unsigned char auc_mmio_sel[NUMBER_OF_MMIOS];
	unsigned char auc_mmio_inv[NUMBER_OF_MMIOS];
} SYSTEM_CONFIG_T;


typedef struct STRUCT_ROMLOADER_OPTIONS
{
	/* console device settings */
	UART_CONFIGURATION_T at_uart[3];
	RAP_UART_CONFIGURATION_T at_rap_uart[4];
	USB_CONFIGURATION_T t_usb;

	/* media settings */
	BOOT_SPI_FLASH_CONFIGURATION_T atSpiFlashCfg[9];

	BOOT_SPI_MMC_CONFIGURATION_T atSpiSdMmcCfg[9];

	/* memory settings */
	NETX_SRAM_CONFIGURATION_T tNetxHifSram;
	NETX_SRAM_CONFIGURATION_T tNetxMemSram;
	NETX_SRAM_CURRENT_CONFIGURATION_T tNetxCurrentSram;

	NETX_SDRAM_CONFIGURATION_T tNetxMemSdram;
	NETX_SDRAM_CONFIGURATION_T tNetxHifSdram;

	NAND_PREDEFINED_DEVICE_PARAMETERS_T tNand;
	NAND_DEVICE_PARAMETERS_T tNandCurrent;

	PL353_NOR_SETTINGS_T tPl353Nor;
	PL353_NOR_SETTINGS_T tPl353NorCurrent;

	NETX_DRAM_CONFIGURATION_T tDdrOptions;

	/* boot settings */
	BOOTDEV_CONFIGURATION_T t_bootdev_options;

	/* console options */
	CONSOLE_IO_OPTIONS_T t_console_options;

	/* SQI options */
	BOOT_SQIROM_CONFIGURATION_T atSqiRomOptions[3];

	/* DPM options */
	HIF_CONFIGURATION_T t_hif_options;
	PCIE_BOOT_OPTIONS_T tPcieOptions;

	/* Ethernet settings */
	ETHERNET_CONFIGURATION_T t_ethernet;

	/* SDIO settings. */
	SDIO_BOOT_OPTIONS_T tSdioOptions;

	/* system configuration */
	SYSTEM_CONFIG_T t_system_config;

	/* I2C configuration. */
	I2C_OPTIONS_T tI2cOptions;

	/* Default Port control. */
	unsigned short ausPortControlDefault[291];
} ROMLOADER_OPTIONS_T;


extern ROMLOADER_OPTIONS_T g_t_romloader_options;


#endif  /* __OPTIONS_H__ */

