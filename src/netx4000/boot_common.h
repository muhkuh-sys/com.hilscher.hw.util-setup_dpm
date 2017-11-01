/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __BOOT_COMMON_H__
#define __BOOT_COMMON_H__


#include "tools.h"


#define BOOTBLOCK_HBOOT_SIGNATURE QUAD_ID('M','O','O','H')


typedef enum ENUM_BOOT_MODE_LED
{
	BootModeLed_REE_SD                 = 0,
	BootModeLed_netX_HIF_DPM_serial    = 1,
	BootModeLed_netX_ETH_LVDS          = 2,
	BootModeLed_minimal_update         = 3,
	BootModeLed_REE_DPM_PCIE           = 4,
	BootModeLed_netX_HIF_DPM_parallel  = 5,
	BootModeLed_REE_PL353_NAND         = 6,
	BootModeLed_netX_MEM_ETH           = 7
} BOOT_MODE_LED_T;


/* Enumerator defining the different boot option.
 * It is used as parameter for the application.
 */
typedef enum ENUM_BOOTOPTION
{
	BOOTOPTION_PFlash_SRAMBus       = 0,
	BOOTOPTION_PFlash_ExtBus        = 1,
	BOOTOPTION_DualPort             = 2,
	BOOTOPTION_PCI                  = 3,
	BOOTOPTION_MMC                  = 4,
	BOOTOPTION_I2C                  = 5,
	BOOTOPTION_SpiFlash             = 6,
	BOOTOPTION_Ethernet             = 7,
} BOOTOPTION_T;

/* Enumerator for all memory devices (i.e. memory areas with "w" attribute).
 * This is used by the HBoot "memory device up" chunk. */
typedef enum ENUM_MEMDEV
{
	MEMDEV_SDRAM_E          = 0,
	MEMDEV_SRAM_E_CS0       = 1,
	MEMDEV_SRAM_E_CS1       = 2,
	MEMDEV_SRAM_E_CS2       = 3,
	MEMDEV_SRAM_E_CS3       = 4,
	MEMDEV_SDRAM_H          = 5,
	MEMDEV_SRAM_H_CS0       = 6,
	MEMDEV_SRAM_H_CS1       = 7,
	MEMDEV_SRAM_H_CS2       = 8,
	MEMDEV_SRAM_H_CS3       = 9,
	MEMDEV_DDR              = 10
} MEMDEV_T;

typedef enum ENUM_BOOTDEV
{
	BOOTDEV_NONE                    = 0x00,

	BOOTDEV_SQI0_FLASH_CS0          = 0x01,
	BOOTDEV_SQI1_FLASH_CS0          = 0x02,

	BOOTDEV_SQI0_SDMMC_CS0          = 0x03,
	BOOTDEV_SQI1_SDMMC_CS0          = 0x04,

	BOOTDEV_SPI0_FLASH_CS0          = 0x05,
	BOOTDEV_SPI0_FLASH_CS1          = 0x06,
	BOOTDEV_SPI0_FLASH_CS2          = 0x07,
	BOOTDEV_SPI1_FLASH_CS0          = 0x08,
	BOOTDEV_SPI1_FLASH_CS1          = 0x09,
	BOOTDEV_SPI1_FLASH_CS2          = 0x0a,

	BOOTDEV_SPI0_SDMMC_CS0          = 0x0b,
	BOOTDEV_SPI0_SDMMC_CS1          = 0x0c,
	BOOTDEV_SPI0_SDMMC_CS2          = 0x0d,
	BOOTDEV_SPI1_SDMMC_CS0          = 0x0e,
	BOOTDEV_SPI1_SDMMC_CS1          = 0x0f,
	BOOTDEV_SPI1_SDMMC_CS2          = 0x10,

	BOOTDEV_PFL_MEM_CS0             = 0x11,
	BOOTDEV_PFL_MEM_CS1             = 0x12,
	BOOTDEV_PFL_MEM_CS2             = 0x13,
	BOOTDEV_PFL_MEM_CS3             = 0x14,

	BOOTDEV_PFL_HIF_CS0             = 0x15,
	BOOTDEV_PFL_HIF_CS1             = 0x16,
	BOOTDEV_PFL_HIF_CS2             = 0x17,
	BOOTDEV_PFL_HIF_CS3             = 0x18,

	BOOTDEV_DPM_PAR                 = 0x19,
	BOOTDEV_DPM_SER                 = 0x1a,

	BOOTDEV_ETH_INTPHY0             = 0x1b,
	BOOTDEV_ETH_INTPHY1             = 0x1c,
	BOOTDEV_ETH_GMAC                = 0x1d,
	BOOTDEV_ETH_LVDS0               = 0x1e,
	BOOTDEV_ETH_LVDS1               = 0x1f,

	BOOTDEV_PFL_NAND                = 0x20,
	BOOTDEV_PFL_PL353_CS0           = 0x21,
	BOOTDEV_PFL_PL353_CS1           = 0x22,

	BOOTDEV_PCIE                    = 0x23,
	BOOTDEV_IDPM0                   = 0x24,

	BOOTDEV_SDIO                    = 0x25,

	BOOTDEV_STOP_SIMU               = 0xff
} BOOTDEV_T;



typedef enum ENUM_BOOTING
{
	BOOTING_Ok				= 0,  /* Booting was OK and the code returned. */
	BOOTING_Not_Allowed			= 1,  /* The configuration does not allow access to this interface. */
	BOOTING_Setup_Error                     = 2,  /* Detection or setup of the media failed. */
	BOOTING_Transfer_Error                  = 3,  /* Transferring the data from the media failed. */
	BOOTING_Cookie_Invalid			= 4,  /* The magic cookie is invalid. */
	BOOTING_Signature_Invalid		= 5,  /* The signature is invalid. */
	BOOTING_Header_Checksum_Invalid		= 6,  /* The header checksum is not correct. */
	BOOTING_Image_Processing_Errors         = 7,  /* Errors occurred while processing the HBOOT image. */
	BOOTING_Secure_Error                    = 8,  /* Something bad with the security stuff happened. */
} BOOTING_T;


typedef struct STRUCT_BOOTDEV_CONFIGURATION
{
	unsigned char auc_boot_devices[8];
} BOOTDEV_CONFIGURATION_T;


typedef int (*PFN_HBOOT_GET_DWORD)(unsigned long *pulData, void *pvUser);
typedef int (*PFN_HBOOT_GET_AREA)(unsigned long *pulData, size_t sizDwords, void *pvUser);
typedef int (*PFN_HBOOT_STOP)(void *pvUser);
typedef int (*PFN_HBOOT_RESTART)(void *pvUser);
typedef int (*PFN_HBOOT_PLAY_SPI_MACRO)(unsigned int uiSpiUnitOffset, const unsigned char *pucMacro, size_t sizMacro, void *pvUser);
typedef int (*PFN_HBOOT_SKIP)(size_t sizDwords, void *pvUser);

typedef struct HBOOT_TRANSPORT_INTERFACE_STRUCT
{
	PFN_HBOOT_GET_DWORD pfnGetDword;
	PFN_HBOOT_GET_AREA pfnGetArea;
	PFN_HBOOT_STOP pfnStop;
	PFN_HBOOT_RESTART pfnRestart;
	PFN_HBOOT_PLAY_SPI_MACRO pfnPlaySpiMacro;
	PFN_HBOOT_SKIP pfnSkip;
} HBOOT_TRANSPORT_INTERFACE_T;


void set_standard_boot_sequence(void);
BOOTING_T boot_device(BOOTDEV_T tDev);
BOOTING_T boot_probe_sequence(void);

BOOTING_T boot_process_hboot_image(const HBOOT_TRANSPORT_INTERFACE_T *ptTransportInterface, void *pvUser, size_t sizHashDwords);

BOOTING_T boot_probe_mmapped_image(const unsigned long *pulImage, PFN_HBOOT_STOP pfnStop, PFN_HBOOT_RESTART pfnRestart, void *pvUser);

void boot_common_init(void);


#endif	/* __BOOT_COMMON_H__ */
