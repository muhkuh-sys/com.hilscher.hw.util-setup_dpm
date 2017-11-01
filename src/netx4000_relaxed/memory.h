/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __MEMORY_H__
#define __MEMORY_H__


typedef struct STRUCT_NETX_SDRAM_CONFIGURATION
{
	unsigned long ul_general_ctrl;
	unsigned long ul_timing_ctrl;
	unsigned long ul_mr;
	unsigned short us_sdram_setup_timeout_ms;
} NETX_SDRAM_CONFIGURATION_T;


typedef struct STRUCT_NETX_SRAM_CONFIGURATION
{
	unsigned long aulCtrl[4];
	unsigned long  ulApmCtrl;
	unsigned long  ulRdyCfg;
} NETX_SRAM_CONFIGURATION_T;


typedef struct STRUCT_NETX_SRAM_CURRENT_CONFIGURATION
{
	unsigned long ulCtrl;
	unsigned long ulApmCtrl;
	unsigned long ulRdyCfg;
} NETX_SRAM_CURRENT_CONFIGURATION_T;


typedef struct STRUCT_NETX_DRAM_CONFIGURATION
{
	unsigned long ulPowerUpTimeoutTicks;
	unsigned char aucScript[2048];
} NETX_DRAM_CONFIGURATION_T;


/* This enumeration lists all possible ways to connect some memory device
 * like thing to a netX.
 */
typedef enum ENUM_MEM_NETX_DEV
{
	MEM_NETX_DEV_None              = 0,  /* No memory device. */

	/* netX MEM interface */
	MEM_NETX_DEV_SRAM_MEM_CS0_08   = 1,  /*  8bit  SRAM @ netX MEM Bus, CS0 */
	MEM_NETX_DEV_SRAM_MEM_CS1_08   = 2,  /*  8bit  SRAM @ netX MEM Bus, CS1 */
	MEM_NETX_DEV_SRAM_MEM_CS2_08   = 3,  /*  8bit  SRAM @ netX MEM Bus, CS2 */
	MEM_NETX_DEV_SRAM_MEM_CS3_08   = 4,  /*  8bit  SRAM @ netX MEM Bus, CS3 */
	MEM_NETX_DEV_SRAM_MEM_CS0_16   = 5,  /* 16bit  SRAM @ netX MEM Bus, CS0 */
	MEM_NETX_DEV_SRAM_MEM_CS1_16   = 6,  /* 16bit  SRAM @ netX MEM Bus, CS1 */
	MEM_NETX_DEV_SRAM_MEM_CS2_16   = 7,  /* 16bit  SRAM @ netX MEM Bus, CS2 */
	MEM_NETX_DEV_SRAM_MEM_CS3_16   = 8,  /* 16bit  SRAM @ netX MEM Bus, CS3 */
	MEM_NETX_DEV_SRAM_MEM_CS0_32   = 9,  /* 32bit  SRAM @ netX MEM Bus, CS0 */
	MEM_NETX_DEV_SRAM_MEM_CS1_32   = 10,  /* 32bit  SRAM @ netX MEM Bus, CS1 */
	MEM_NETX_DEV_SRAM_MEM_CS2_32   = 11,  /* 32bit  SRAM @ netX MEM Bus, CS2 */

	MEM_NETX_DEV_SDRAM_MEM_08      = 12,  /*  8bit SDRAM @ netX MEM Interface */
	MEM_NETX_DEV_SDRAM_MEM_16      = 13,  /* 16bit SDRAM @ netX MEM Interface */
	MEM_NETX_DEV_SDRAM_MEM_32      = 14,  /* 32bit SDRAM @ netX MEM Interface */


	/* netX HIF interface */
	MEM_NETX_DEV_DPM               = 15,  /* DPM Interface */

	MEM_NETX_DEV_SRAM_HIF_CS0_08   = 16,  /*  8bit  SRAM @ netX HIF Bus, CS0 */
	MEM_NETX_DEV_SRAM_HIF_CS1_08   = 17,  /*  8bit  SRAM @ netX HIF Bus, CS1 */
	MEM_NETX_DEV_SRAM_HIF_CS2_08   = 18,  /*  8bit  SRAM @ netX HIF Bus, CS2 */
	MEM_NETX_DEV_SRAM_HIF_CS3_08   = 19,  /*  8bit  SRAM @ netX HIF Bus, CS3 */
	MEM_NETX_DEV_SRAM_HIF_CS0_16   = 20,  /* 16bit  SRAM @ netX HIF Bus, CS0 */
	MEM_NETX_DEV_SRAM_HIF_CS1_16   = 21,  /* 16bit  SRAM @ netX HIF Bus, CS1 */
	MEM_NETX_DEV_SRAM_HIF_CS2_16   = 22,  /* 16bit  SRAM @ netX HIF Bus, CS2 */
	MEM_NETX_DEV_SRAM_HIF_CS3_16   = 23,  /* 16bit  SRAM @ netX HIF Bus, CS3 */
	MEM_NETX_DEV_SRAM_HIF_CS0_32   = 24,  /* 32bit  SRAM @ netX HIF Bus, CS0 */
	MEM_NETX_DEV_SRAM_HIF_CS1_32   = 25,  /* 32bit  SRAM @ netX HIF Bus, CS1 */

	MEM_NETX_DEV_SDRAM_HIF_08      = 26,  /*  8bit SDRAM @ netX HIF Interface */
	MEM_NETX_DEV_SDRAM_HIF_16      = 27,  /* 16bit SDRAM @ netX HIF Interface */
	MEM_NETX_DEV_SDRAM_HIF_32      = 28   /* 32bit SDRAM @ netX HIF Interface */
} MEM_NETX_DEV_T;



typedef enum ENUM_MEM_RAP_DEV
{
	MEM_RAP_DEV_None              = 0,  /* No memory device. */

	/* RAP PL353 interface */
	MEM_RAP_DEV_NAND08            = 1,
	MEM_RAP_DEV_NAND16            = 2,
	MEM_RAP_DEV_SRAM_PL353_CS0_08 = 3,
	MEM_RAP_DEV_SRAM_PL353_CS1_08 = 4,
	MEM_RAP_DEV_SRAM_PL353_CS0_16 = 5,
	MEM_RAP_DEV_SRAM_PL353_CS1_16 = 6,

	MEM_RAP_DEV_DDR               = 7
} MEM_RAP_DEV_T;



typedef enum DPM_IRQFIQ_CFG_ENUM
{
	DPM_IRQFIQ_CFG_Fiq_DPM_Sw		= 0x01,
	DPM_IRQFIQ_CFG_Fiq_DPM_Err		= 0x02,
	DPM_IRQFIQ_CFG_Fiq_Firmware		= 0x04,
	DPM_IRQFIQ_CFG_Irq_DPM_Sw		= 0x08,
	DPM_IRQFIQ_CFG_Irq_DPM_Err		= 0x10,
	DPM_IRQFIQ_CFG_Irq_Firmware		= 0x20
} DPM_IRQFIQ_CFG_T;


typedef enum PCIE_IRQ_CFG_ENUM
{
	PCIE_INTA_CFG_DPM_Sw		= 0x01,
	PCIE_INTA_CFG_Firmware		= 0x02,
	PCIE_INTB_CFG_DPM_Sw		= 0x04,
	PCIE_INTB_CFG_Firmware		= 0x08,
	PCIE_INTC_CFG_DPM_Sw		= 0x10,
	PCIE_INTC_CFG_Firmware		= 0x20,
	PCIE_INTD_CFG_DPM_Sw		= 0x40,
	PCIE_INTD_CFG_Firmware		= 0x80
} PCIE_IRQ_CFG_T;


typedef struct STRUCT_HIF_CONFIGURATION
{
	unsigned long   ulHifIoCfg;
	unsigned long   ulFirmwareIrqMask;
	unsigned char   ucDpmCfg0x0;
	unsigned char   ucDpmAddrCfg;
	unsigned char   ucDpmTimingCfg;
	unsigned char   ucDpmRdyCfg;
	unsigned char   ucDpmMiscCfg;
	unsigned char   ucDpmIoCfgMisc;
	unsigned char   ucDpmIrqFiqCfg;
	unsigned char   ucPcieIntCfg;
	unsigned short ausPortControl[84];
} HIF_CONFIGURATION_T;


typedef enum MEMORY_INTERFACE_ENUM
{
	MEMORY_INTERFACE_MEM_SRAM     = 0,
	MEMORY_INTERFACE_MEM_SDRAM    = 1,
	MEMORY_INTERFACE_HIF_SRAM     = 2,
	MEMORY_INTERFACE_HIF_SDRAM    = 3,
	MEMORY_INTERFACE_PL353_NAND   = 4,
	MEMORY_INTERFACE_PL353_SRAM   = 5,
	MEMORY_INTERFACE_DDR          = 6
} MEMORY_INTERFACE_T;


typedef enum MEMORY_WIDTH_ENUM
{
	MEMORY_WIDTH_Off    = 0,
	MEMORY_WIDTH_08Bit  = 1,
	MEMORY_WIDTH_16Bit  = 2,
	MEMORY_WIDTH_32Bit  = 3,
	MEMORY_WIDTH_Auto   = 4
} MEMORY_WIDTH_T;



typedef enum DDR_SETUP_COMMAND_ENUM
{
	DDR_SETUP_COMMAND_Nop         = 0,
	DDR_SETUP_COMMAND_WritePhy    = 1,
	DDR_SETUP_COMMAND_WriteCtrl   = 2,
	DDR_SETUP_COMMAND_DelayTicks  = 3,
	DDR_SETUP_COMMAND_PollPhy     = 4,
	DDR_SETUP_COMMAND_PollCtrl    = 5
} DDR_SETUP_COMMAND_T;



MEM_NETX_DEV_T memory_setup_netx_sram(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect, MEMORY_WIDTH_T tBusWidth);

unsigned long *memory_get_base_pointer_netx(MEM_NETX_DEV_T tMemDev);
unsigned long *memory_get_base_pointer_rap(MEM_RAP_DEV_T tMemDev);

void memory_deactivate_netx_sram(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect);

void setup_hif_and_mem_portctrl(void);



//void set_default_memory_policy(unsigned int uiIndex);

int memory_setup_sdram(MEMORY_INTERFACE_T tInterface);
unsigned long memory_get_sdram_size(unsigned long ulGeneralCtrl);
int memory_setup_sram(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect, unsigned long ulSRamCtrl);

void memory_deactivate_all_devices(void);

int memory_setup_ddr(void);


#endif	/* __MEMORY_H__ */
