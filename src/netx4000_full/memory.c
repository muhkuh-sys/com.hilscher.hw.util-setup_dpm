/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include "memory.h"

#include <string.h>

#include "cr7_global_timer.h"
#include "netx_io_areas.h"
#include "options.h"
#include "portcontrol.h"

/* Do not use the real OTP fuses on the SCIT board. They can not be defined
 * completely and will trigger a security error in all cases.
 */
#if CFG_ARTIFICIAL_OTP_FUSES!=0
#       include "otp_fuses_scit_board.h"
#endif



static const MEM_NETX_DEV_T atAvailableMemNetxDevices[3*4*4] =
{
/* MEMORY_INTERFACE_MEM_SRAM */
	/* 08 */                        /* 16 */                        /* 32 */
/*CS0*/ MEM_NETX_DEV_SRAM_MEM_CS0_08,   MEM_NETX_DEV_SRAM_MEM_CS0_16,   MEM_NETX_DEV_SRAM_MEM_CS0_32,
/*CS1*/ MEM_NETX_DEV_SRAM_MEM_CS1_08,   MEM_NETX_DEV_SRAM_MEM_CS1_16,   MEM_NETX_DEV_None,
/*CS2*/ MEM_NETX_DEV_SRAM_MEM_CS2_08,   MEM_NETX_DEV_SRAM_MEM_CS2_16,   MEM_NETX_DEV_SRAM_MEM_CS2_32,
/*CS3*/ MEM_NETX_DEV_SRAM_MEM_CS3_08,   MEM_NETX_DEV_SRAM_MEM_CS3_16,   MEM_NETX_DEV_None,

/* MEMORY_INTERFACE_MEM_SDRAM */
	/* 08 */                        /* 16 */                        /* 32 */
/*CS0*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_SDRAM_MEM_16,      MEM_NETX_DEV_SDRAM_MEM_32,
/*CS1*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_None,              MEM_NETX_DEV_None,
/*CS2*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_None,              MEM_NETX_DEV_None,
/*CS3*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_None,              MEM_NETX_DEV_None,

/* MEMORY_INTERFACE_HIF_SRAM */
	/* 08 */                        /* 16 */                        /* 32 */
/*CS0*/ MEM_NETX_DEV_SRAM_HIF_CS0_08,   MEM_NETX_DEV_SRAM_HIF_CS0_16,   MEM_NETX_DEV_SRAM_HIF_CS0_32,
/*CS1*/ MEM_NETX_DEV_SRAM_HIF_CS1_08,   MEM_NETX_DEV_SRAM_HIF_CS1_16,   MEM_NETX_DEV_SRAM_HIF_CS1_32,
/*CS2*/ MEM_NETX_DEV_SRAM_HIF_CS2_08,   MEM_NETX_DEV_SRAM_HIF_CS2_16,   MEM_NETX_DEV_None,
/*CS3*/ MEM_NETX_DEV_SRAM_HIF_CS3_08,   MEM_NETX_DEV_SRAM_HIF_CS3_16,   MEM_NETX_DEV_None,

/* MEMORY_INTERFACE_HIF_SDRAM */
	/* 08 */                        /* 16 */                        /* 32 */
/*CS0*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_SDRAM_HIF_16,      MEM_NETX_DEV_SDRAM_HIF_32,
/*CS1*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_None,              MEM_NETX_DEV_None,
/*CS2*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_None,              MEM_NETX_DEV_None,
/*CS3*/ MEM_NETX_DEV_None,              MEM_NETX_DEV_None,              MEM_NETX_DEV_None,
};



/* Check if an SRam device with a specific data width is possible on this
 * specific hardware.
 *
 * Parameters are:
 *   tUnit: MEMORY_UNIT_EXT_BUS (0)  Extension bus
 *          MEMORY_UNIT_HIF_BUS (1)  Host Interface
 *   uiChipSelect: 0..3
 *   uiDWidth: 0 =  8 bit
 *             1 = 16 bit
 *             2 = 32 bit
 *             3 = disabled
 */
static MEM_NETX_DEV_T is_netx_device_allowed(MEMORY_INTERFACE_T tIf, unsigned int uiChipSelect, MEMORY_WIDTH_T tWidth)
{
	MEM_NETX_DEV_T tMemDev;
	unsigned int uiIf;
	unsigned int uiWidth;
	unsigned int uiOffset;


	/* Be pessimistic. */
	tMemDev = MEM_NETX_DEV_None;

	uiIf = (unsigned int)tIf;

	uiWidth = 4;
	switch(tWidth)
	{
	case MEMORY_WIDTH_Off:
		break;

	case MEMORY_WIDTH_08Bit:
		uiWidth = 0;
		break;

	case MEMORY_WIDTH_16Bit:
		uiWidth = 1;
		break;

	case MEMORY_WIDTH_32Bit:
		uiWidth = 2;
		break;

	case MEMORY_WIDTH_Auto:
		break;
	}

	if( uiIf<8U && uiChipSelect<4U && uiWidth<3U )
	{
		uiOffset = uiWidth + (3U*uiChipSelect) + (3U*4U*uiIf);
		if( uiOffset<(sizeof(atAvailableMemNetxDevices)/sizeof(atAvailableMemNetxDevices[0])) )
		{
			tMemDev = atAvailableMemNetxDevices[uiOffset];
		}

	}

	return tMemDev;
}



MEM_NETX_DEV_T memory_setup_netx_sram(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect, MEMORY_WIDTH_T tBusWidth)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptExtAsyncmemCtrlArea);
	HOSTDEF(ptHifAsyncmemCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	unsigned long ulDWidth;
	unsigned long ulMiCfg;
	unsigned long ulValue;
	MEM_NETX_DEV_T tMemDev;
	HOSTADEF(EXT_ASYNCMEM_CTRL) *ptSRAMController;


	tMemDev = MEM_NETX_DEV_None;

	if( uiChipSelect<4 )
	{
		ptSRAMController = NULL;
		switch(tInterface)
		{
		case MEMORY_INTERFACE_MEM_SRAM:
			ptSRAMController = ptExtAsyncmemCtrlArea;
			break;

		case MEMORY_INTERFACE_MEM_SDRAM:
			break;

		case MEMORY_INTERFACE_HIF_SRAM:
			ptSRAMController = ptHifAsyncmemCtrlArea;
			break;

		case MEMORY_INTERFACE_HIF_SDRAM:
			break;

		case MEMORY_INTERFACE_PL353_NAND:
			break;

		case MEMORY_INTERFACE_PL353_SRAM:
			break;

		case MEMORY_INTERFACE_DDR:
			break;
		}

		if( ptSRAMController!=NULL )
		{
			tMemDev = is_netx_device_allowed(tInterface, uiChipSelect, tBusWidth);
			if( tMemDev!=MEM_NETX_DEV_None )
			{
				ulDWidth = 0;
				ulMiCfg = 0;
				switch( tBusWidth )
				{
				case MEMORY_WIDTH_Off:
					break;

				case MEMORY_WIDTH_08Bit:
					break;

				case MEMORY_WIDTH_16Bit:
					ulDWidth = 1;
					ulMiCfg = 1;
					break;

				case MEMORY_WIDTH_32Bit:
					ulDWidth = 2;
					ulMiCfg = 2;
					break;

				case MEMORY_WIDTH_Auto:
					break;
				}
				ulValue  = g_t_romloader_options.tNetxCurrentSram.ulCtrl;
				ulValue &= ~HOSTMSK(extsram0_ctrl_dwidth);
				ulValue |= ulDWidth << HOSTSRT(extsram0_ctrl_dwidth);
				//trace_message_ul(TRACEMSG_Memory_SRAM_Control, ulValue);
				ptSRAMController->aulExtsram_ctrl[uiChipSelect] = ulValue;

				/* Set the HIF IO configuration. */
				ulValue  = g_t_romloader_options.t_hif_options.ulHifIoCfg;
				if( tInterface==MEMORY_INTERFACE_MEM_SRAM )
				{
					ulValue &= ~HOSTMSK(hif_io_cfg_mem_mi_cfg);
					ulValue |= ulMiCfg << HOSTSRT(hif_io_cfg_mem_mi_cfg);
				}
				else
				{
					ulValue &= ~HOSTMSK(hif_io_cfg_hif_mi_cfg);
					ulValue |= ulMiCfg << HOSTSRT(hif_io_cfg_hif_mi_cfg);
				}
				//trace_message_ul(TRACEMSG_Memory_hif_io_cfg, ulValue);
				__IRQ_LOCK__;
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
				ptHifIoCtrlArea->ulHif_io_cfg = ulValue;
				__IRQ_UNLOCK__;

				/* Set the port control configuration for all HIF and MEM pins. */
				setup_hif_and_mem_portctrl();
			}
		}
	}

	return tMemDev;
}



unsigned long *memory_get_base_pointer_netx(MEM_NETX_DEV_T tMemDev)
{
	unsigned long *pulResult;


	pulResult = NULL;
	switch(tMemDev)
	{
	case MEM_NETX_DEV_None:
		break;

	case MEM_NETX_DEV_SRAM_MEM_CS0_08:
	case MEM_NETX_DEV_SRAM_MEM_CS0_16:
	case MEM_NETX_DEV_SRAM_MEM_CS0_32:
		pulResult = (unsigned long*)(HOSTADR(mem_extsram_cs0_base));
		break;

	case MEM_NETX_DEV_SRAM_MEM_CS1_08:
	case MEM_NETX_DEV_SRAM_MEM_CS1_16:
	case MEM_NETX_DEV_SRAM_MEM_CS1_32:
		pulResult = (unsigned long*)(HOSTADR(mem_extsram_cs1_base));
		break;

	case MEM_NETX_DEV_SRAM_MEM_CS2_08:
	case MEM_NETX_DEV_SRAM_MEM_CS2_16:
	case MEM_NETX_DEV_SRAM_MEM_CS2_32:
		pulResult = (unsigned long*)(HOSTADR(mem_extsram_cs2_base));
		break;

	case MEM_NETX_DEV_SRAM_MEM_CS3_08:
	case MEM_NETX_DEV_SRAM_MEM_CS3_16:
		pulResult = (unsigned long*)(HOSTADR(mem_extsram_cs3_base));
		break;

	case MEM_NETX_DEV_SDRAM_MEM_08:
	case MEM_NETX_DEV_SDRAM_MEM_16:
	case MEM_NETX_DEV_SDRAM_MEM_32:
		pulResult = (unsigned long*)(HOSTADR(mem_sdram_sdram_base));
		break;

	case MEM_NETX_DEV_DPM:
		break;

	case MEM_NETX_DEV_SRAM_HIF_CS0_08:
	case MEM_NETX_DEV_SRAM_HIF_CS0_16:
	case MEM_NETX_DEV_SRAM_HIF_CS0_32:
		pulResult = (unsigned long*)(HOSTADR(hif_extsram_cs0_base));
		break;

	case MEM_NETX_DEV_SRAM_HIF_CS1_08:
	case MEM_NETX_DEV_SRAM_HIF_CS1_16:
	case MEM_NETX_DEV_SRAM_HIF_CS1_32:
		pulResult = (unsigned long*)(HOSTADR(hif_extsram_cs1_base));
		break;

	case MEM_NETX_DEV_SRAM_HIF_CS2_08:
	case MEM_NETX_DEV_SRAM_HIF_CS2_16:
		pulResult = (unsigned long*)(HOSTADR(hif_extsram_cs2_base));
		break;

	case MEM_NETX_DEV_SRAM_HIF_CS3_08:
	case MEM_NETX_DEV_SRAM_HIF_CS3_16:
		pulResult = (unsigned long*)(HOSTADR(hif_extsram_cs3_base));
		break;

	case MEM_NETX_DEV_SDRAM_HIF_08:
	case MEM_NETX_DEV_SDRAM_HIF_16:
	case MEM_NETX_DEV_SDRAM_HIF_32:
		pulResult = (unsigned long*)(HOSTADR(hif_sdram_sdram_base));
		break;
	}

	return pulResult;
}



unsigned long *memory_get_base_pointer_rap(MEM_RAP_DEV_T tMemDev)
{
	unsigned long *pulResult;


	pulResult = NULL;
	switch(tMemDev)
	{
	case MEM_RAP_DEV_None:
		break;

	case MEM_RAP_DEV_NAND08:
	case MEM_RAP_DEV_NAND16:
		pulResult = (unsigned long*)(HOSTADDR(NX2RAP_nflash));
		break;

	case MEM_RAP_DEV_SRAM_PL353_CS0_08:
	case MEM_RAP_DEV_SRAM_PL353_CS0_16:
		pulResult = (unsigned long*)(HOSTADDR(NX2RAP_extsram0));
		break;

	case MEM_RAP_DEV_SRAM_PL353_CS1_08:
	case MEM_RAP_DEV_SRAM_PL353_CS1_16:
		pulResult = (unsigned long*)(HOSTADDR(NX2RAP_extsram1));
		break;

	case MEM_RAP_DEV_DDR:
		break;
	}

	return pulResult;
}



void memory_deactivate_netx_sram(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptExtAsyncmemCtrlArea);
	HOSTDEF(ptHifAsyncmemCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	unsigned long ulValue;


	if( uiChipSelect<4 )
	{
		switch(tInterface)
		{
		case MEMORY_INTERFACE_MEM_SRAM:
			ulValue = 3U << HOSTSRT(extsram0_ctrl_dwidth);
			ptExtAsyncmemCtrlArea->aulExtsram_ctrl[uiChipSelect] = ulValue;

			ulValue  = g_t_romloader_options.t_hif_options.ulHifIoCfg;
			ulValue &= ~HOSTMSK(hif_io_cfg_mem_mi_cfg);
			ulValue |= 3U << HOSTSRT(hif_io_cfg_mem_mi_cfg);
			__IRQ_LOCK__;
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
			ptHifIoCtrlArea->ulHif_io_cfg = ulValue;
			__IRQ_UNLOCK__;

			break;

		case MEMORY_INTERFACE_MEM_SDRAM:
			break;

		case MEMORY_INTERFACE_HIF_SRAM:
			ulValue = 3U << HOSTSRT(extsram0_ctrl_dwidth);
			ptHifAsyncmemCtrlArea->aulExtsram_ctrl[uiChipSelect] = ulValue;

			ulValue  = g_t_romloader_options.t_hif_options.ulHifIoCfg;
			ulValue &= ~HOSTMSK(hif_io_cfg_hif_mi_cfg);
			ulValue |= 3U << HOSTSRT(hif_io_cfg_hif_mi_cfg);
			__IRQ_LOCK__;
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
			ptHifIoCtrlArea->ulHif_io_cfg = ulValue;
			__IRQ_UNLOCK__;


			break;

		case MEMORY_INTERFACE_HIF_SDRAM:
			break;

		case MEMORY_INTERFACE_PL353_NAND:
			break;

		case MEMORY_INTERFACE_PL353_SRAM:
			break;

		case MEMORY_INTERFACE_DDR:
			break;
		}
	}
}



int memory_setup_sdram(MEMORY_INTERFACE_T tInterface)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMemSdramArea);
	HOSTDEF(ptHifIoCtrlArea);
	HOSTDEF(ptHifSdramArea);
	NETX_SDRAM_CONFIGURATION_T *ptCfg;
	unsigned long ulGeneralCtrl;
	unsigned long ulValue;
	TIMER_HANDLE_T tHandle;
	int iResult;
	HOSTADEF(EXT_SDRAM_CTRL) *ptSDRAMController;
	NETX_SDRAM_CONFIGURATION_T tNetxCurrentSdram;


	/* Expect error. */
	iResult = -1;

	/* Select the configuration. */
	ptCfg = NULL;
	ptSDRAMController = NULL;
	switch(tInterface)
	{
	case MEMORY_INTERFACE_MEM_SRAM:
		break;

	case MEMORY_INTERFACE_MEM_SDRAM:
		ptCfg = &(g_t_romloader_options.tNetxMemSdram);
		ptSDRAMController = ptMemSdramArea;
		break;

	case MEMORY_INTERFACE_HIF_SRAM:
		break;

	case MEMORY_INTERFACE_HIF_SDRAM:
		ptCfg = &(g_t_romloader_options.tNetxHifSdram);
		ptSDRAMController = ptHifSdramArea;
		break;

	case MEMORY_INTERFACE_PL353_NAND:
		break;

	case MEMORY_INTERFACE_PL353_SRAM:
		break;

	case MEMORY_INTERFACE_DDR:
		break;
	}

	if( ptCfg!=NULL )
	{
		/* Initialize the current configuration. */
		memcpy(&tNetxCurrentSdram, ptCfg, sizeof(NETX_SDRAM_CONFIGURATION_T));

		/* Get the general control value. */
		ulGeneralCtrl = tNetxCurrentSdram.ul_general_ctrl;

		/* Is the configuration valid? The activate bit must be set. */
		if( (ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_ctrl_en))!=0 )
		{
			/* Disable the SDRAM controller. */
			ptSDRAMController->ulSdram_general_ctrl = 0;

			/* Configure the HIF pins. */
			ulValue  = g_t_romloader_options.t_hif_options.ulHifIoCfg;
			if( tInterface==MEMORY_INTERFACE_MEM_SDRAM )
			{
				/* Enable an SDRAM on the MEM pins. */
				ulValue |= HOSTMSK(hif_io_cfg_en_mem_sdram_mi);
				/* Set the bus width from the general control value. */
				ulValue &= ~HOSTMSK(hif_io_cfg_mem_mi_cfg);
				if( (ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_dbus32))==0 )
				{
					/* The SDRAM general control register selects 16 bits. */
					ulValue |= 1U << HOSTSRT(hif_io_cfg_mem_mi_cfg);
				}
				else
				{
					/* The SDRAM general control register selects 32 bits. */
					ulValue |= 2U << HOSTSRT(hif_io_cfg_mem_mi_cfg);
				}
			}
			else
			{
				/* Enable an SDRAM on the HIF pins. */
				ulValue |= HOSTMSK(hif_io_cfg_en_hif_sdram_mi);
				/* Set the bus width from the general ctrl value. */
				ulValue &= ~HOSTMSK(hif_io_cfg_hif_mi_cfg);
				if( (ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_dbus32))==0 )
				{
					/* The SDRAM general control register selects 16 bits. */
					ulValue |= 1U << HOSTSRT(hif_io_cfg_hif_mi_cfg);
				}
				else
				{
					/* The SDRAM general control register selects 32 bits. */
					ulValue |= 2U << HOSTSRT(hif_io_cfg_hif_mi_cfg);
				}
			}
			__IRQ_LOCK__;
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
			ptHifIoCtrlArea->ulHif_io_cfg = ulValue;
			__IRQ_UNLOCK__;

			/* Set the port control configuration for all HIF and MEM pins. */
			setup_hif_and_mem_portctrl();

			/* Set the timing control. */
			ptSDRAMController->ulSdram_timing_ctrl = tNetxCurrentSdram.ul_timing_ctrl;
			/* Set the MR register. */
			ptSDRAMController->ulSdram_mr = tNetxCurrentSdram.ul_mr;

			/* Set the general control. */
			ptSDRAMController->ulSdram_general_ctrl = ulGeneralCtrl;

			/* Wait until SDRAM controller is ready. */
			cr7_global_timer_start_ms(&tHandle, tNetxCurrentSdram.us_sdram_setup_timeout_ms);
			do
			{
				ulValue  = ptSDRAMController->ulSdram_general_ctrl;
				ulValue &= HOSTMSK(sdram_general_ctrl_sdram_ready);
				if( ulValue!=0 )
				{
					iResult = 0;
					break;
				}
			} while( cr7_global_timer_elapsed(&tHandle)==0 );

			if( iResult!=0 )
			{
				/* Disable the SDRAM controller. */
				ptSDRAMController->ulSdram_general_ctrl = 0;
			}
		}
	}

	return iResult;
}



/* This sets up an SRAM as a boot destination.
 * It does not change the tNetxCurrentSram configuration in the global options. This is reserved for the boot sources.
 */
int memory_setup_sram(MEMORY_INTERFACE_T tInterface, unsigned int uiChipSelect, unsigned long ulSRamCtrl)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptExtAsyncmemCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	HOSTDEF(ptHifAsyncmemCtrlArea);
	int iResult;
	unsigned int uiMode;
	NETX_SRAM_CONFIGURATION_T *ptCfg;
	HOSTADEF(EXT_ASYNCMEM_CTRL) *ptSRAMController;
	NETX_SRAM_CURRENT_CONFIGURATION_T tNetxCurrentSram;
	MEM_NETX_DEV_T tMemDev;
	MEMORY_WIDTH_T tWidth;


	/* Be pessimistic. */
	iResult = -1;

	if( uiChipSelect<4 )
	{
		uiMode = (ulSRamCtrl & HOSTMSK(extsram0_ctrl_dwidth)) >> HOSTSRT(extsram0_ctrl_dwidth);
		if( uiMode==0 )
		{
			tWidth = MEMORY_WIDTH_08Bit;
		}
		else if( uiMode==1 )
		{
			tWidth = MEMORY_WIDTH_16Bit;
		}
		else if( uiMode==2 )
		{
			tWidth = MEMORY_WIDTH_32Bit;
		}
		else
		{
			tWidth = MEMORY_WIDTH_Off;
		}

		tMemDev = is_netx_device_allowed(tInterface, uiChipSelect, tWidth);
		if( tMemDev!=MEM_NETX_DEV_None )
		{
			ptCfg = NULL;
			ptSRAMController = NULL;
			switch(tInterface)
			{
			case MEMORY_INTERFACE_MEM_SRAM:
				ptCfg = &(g_t_romloader_options.tNetxMemSram);
				ptSRAMController = ptExtAsyncmemCtrlArea;
				break;

			case MEMORY_INTERFACE_MEM_SDRAM:
				break;

			case MEMORY_INTERFACE_HIF_SRAM:
				ptCfg = &(g_t_romloader_options.tNetxHifSram);
				ptSRAMController = ptHifAsyncmemCtrlArea;
				break;

			case MEMORY_INTERFACE_HIF_SDRAM:
				break;

			case MEMORY_INTERFACE_PL353_NAND:
				break;

			case MEMORY_INTERFACE_PL353_SRAM:
				break;

			case MEMORY_INTERFACE_DDR:
				break;
			}

			if( ptCfg!=NULL )
			{
				/* Initialize the current configuration. */
				tNetxCurrentSram.ulCtrl    = ptCfg->aulCtrl[uiChipSelect];
				tNetxCurrentSram.ulApmCtrl = ptCfg->ulApmCtrl;
				tNetxCurrentSram.ulRdyCfg  = ptCfg->ulRdyCfg;

				/* Disable the area in the SRAM controller. */
				ptSRAMController->aulExtsram_ctrl[uiChipSelect] = 3 << HOSTSRT(extsram0_ctrl_dwidth);

				/* Set the general SRAM configuration. */
				ptSRAMController->ulExt_cs0_apm_ctrl = tNetxCurrentSram.ulApmCtrl;
				ptSRAMController->ulExt_rdy_cfg      = tNetxCurrentSram.ulRdyCfg;

				/* Setup the area in the controller. */
				ptSRAMController->aulExtsram_ctrl[uiChipSelect] = ulSRamCtrl;

				/* Configure the HIF pins. */
				__IRQ_LOCK__;
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
				ptHifIoCtrlArea->ulHif_io_cfg = g_t_romloader_options.t_hif_options.ulHifIoCfg;
				__IRQ_UNLOCK__;

				/* Set the port control configuration for all HIF and MEM pins. */
				setup_hif_and_mem_portctrl();

				/* All OK! */
				iResult = 0;
			}
		}
	}

	return iResult;
}



unsigned long memory_get_sdram_size(unsigned long ulGeneralCtrl)
{
	unsigned long ulSize;
	unsigned int uiBusFactor;


	/* netx4000 has 16 or 32 bit bus. */
	uiBusFactor = ((ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_dbus32))!=0) ? 4U : 2U;

	ulSize = (2U    << ((ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_banks))>>HOSTSRT(sdram_general_ctrl_banks))) *
	         (256U  << ((ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_columns))>>HOSTSRT(sdram_general_ctrl_columns))) *
	         (2048U << ((ulGeneralCtrl&HOSTMSK(sdram_general_ctrl_rows))>>HOSTSRT(sdram_general_ctrl_rows))) *
	         uiBusFactor;

	return ulSize;
}



void memory_deactivate_all_devices(void)
{
	HOSTDEF(ptPL353Area);
	unsigned long ulValue;


	/* Set the PL353 into low-power mode. */
	ulValue  = HOSTMSK(PL353_mem_cfg_set_low_power_req);
	ptPL353Area->ulPL353_mem_cfg_set = ulValue;
}



static const unsigned short ausPortControlIndex_HifMem[84] =
{
	PORTCONTROL_INDEX( 9,  1),      // HIF_A0
	PORTCONTROL_INDEX( 9,  2),      // HIF_A1
	PORTCONTROL_INDEX( 9,  3),      // HIF_A2
	PORTCONTROL_INDEX( 9,  4),      // HIF_A3
	PORTCONTROL_INDEX( 9,  5),      // HIF_A4
	PORTCONTROL_INDEX( 9,  6),      // HIF_A5
	PORTCONTROL_INDEX( 9,  7),      // HIF_A6
	PORTCONTROL_INDEX( 9,  8),      // HIF_A7
	PORTCONTROL_INDEX( 9,  9),      // HIF_A8
	PORTCONTROL_INDEX( 9, 10),      // HIF_A9
	PORTCONTROL_INDEX( 9, 11),      // HIF_A10
	PORTCONTROL_INDEX( 9, 12),      // HIF_A11
	PORTCONTROL_INDEX( 9, 13),      // HIF_A12
	PORTCONTROL_INDEX( 9, 14),      // HIF_A13
	PORTCONTROL_INDEX( 9, 15),      // HIF_A14
	PORTCONTROL_INDEX(10,  0),      // HIF_A15
	PORTCONTROL_INDEX(10,  1),      // HIF_A16
	PORTCONTROL_INDEX( 7,  0),      // HIF_A17
	PORTCONTROL_INDEX( 7,  1),      // HIF_D0
	PORTCONTROL_INDEX( 7,  2),      // HIF_D1
	PORTCONTROL_INDEX( 7,  3),      // HIF_D2
	PORTCONTROL_INDEX( 7,  4),      // HIF_D3
	PORTCONTROL_INDEX( 7,  5),      // HIF_D4
	PORTCONTROL_INDEX( 7,  6),      // HIF_D5
	PORTCONTROL_INDEX( 7,  7),      // HIF_D6
	PORTCONTROL_INDEX( 7,  8),      // HIF_D7
	PORTCONTROL_INDEX( 7,  9),      // HIF_D8
	PORTCONTROL_INDEX( 7, 10),      // HIF_D9
	PORTCONTROL_INDEX( 7, 11),      // HIF_D10
	PORTCONTROL_INDEX( 7, 12),      // HIF_D11
	PORTCONTROL_INDEX( 7, 13),      // HIF_D12
	PORTCONTROL_INDEX( 7, 14),      // HIF_D13
	PORTCONTROL_INDEX( 7, 15),      // HIF_D14
	PORTCONTROL_INDEX( 8,  0),      // HIF_D15
	PORTCONTROL_INDEX( 8,  1),      // HIF_D16
	PORTCONTROL_INDEX( 8,  2),      // HIF_D17
	PORTCONTROL_INDEX( 8,  3),      // HIF_D18
	PORTCONTROL_INDEX( 8,  4),      // HIF_D19
	PORTCONTROL_INDEX( 8,  5),      // HIF_D20
	PORTCONTROL_INDEX( 8,  6),      // HIF_D21
	PORTCONTROL_INDEX( 8,  7),      // HIF_D22
	PORTCONTROL_INDEX( 8,  8),      // HIF_D23
	PORTCONTROL_INDEX( 8,  9),      // HIF_D24
	PORTCONTROL_INDEX( 8, 10),      // HIF_D25
	PORTCONTROL_INDEX( 8, 11),      // HIF_D26
	PORTCONTROL_INDEX( 8, 12),      // HIF_D27
	PORTCONTROL_INDEX( 8, 13),      // HIF_D28
	PORTCONTROL_INDEX( 8, 14),      // HIF_D29
	PORTCONTROL_INDEX( 8, 15),      // HIF_D30
	PORTCONTROL_INDEX(10,  2),      // HIF_D31
	PORTCONTROL_INDEX(10,  3),      // HIF_BHE1
	PORTCONTROL_INDEX(10,  4),      // HIF_BHE3
	PORTCONTROL_INDEX(10,  5),      // HIF_CSN
	PORTCONTROL_INDEX(10,  6),      // HIF_RDN
	PORTCONTROL_INDEX(10,  7),      // HIF_WRN
	PORTCONTROL_INDEX(10,  8),      // HIF_RDY
	PORTCONTROL_INDEX(10,  9),      // HIF_DIRQ
	PORTCONTROL_INDEX(10, 10),      // HIF_SDCLK

	PORTCONTROL_INDEX(15,  0),      // MEM_A0
	PORTCONTROL_INDEX(15,  1),      // MEM_A1
	PORTCONTROL_INDEX(15,  2),      // MEM_A2
	PORTCONTROL_INDEX(15,  3),      // MEM_A3
	PORTCONTROL_INDEX(15,  4),      // MEM_A4
	PORTCONTROL_INDEX(15,  5),      // MEM_A5
	PORTCONTROL_INDEX(15,  6),      // MEM_A6
	PORTCONTROL_INDEX(15,  7),      // MEM_A7
	PORTCONTROL_INDEX(15,  8),      // MEM_A8
	PORTCONTROL_INDEX(15,  9),      // MEM_A9
	PORTCONTROL_INDEX(15, 10),      // MEM_A10
	PORTCONTROL_INDEX(15, 11),      // MEM_A11
	PORTCONTROL_INDEX(15, 12),      // MEM_A12
	PORTCONTROL_INDEX(15, 13),      // MEM_A13
	PORTCONTROL_INDEX(16,  0),      // MEM_A14
	PORTCONTROL_INDEX(16,  1),      // MEM_A15
	PORTCONTROL_INDEX(16,  2),      // MEM_A16
	PORTCONTROL_INDEX(16,  3),      // MEM_A17
	PORTCONTROL_INDEX(16,  4),      // MEM_BHE1
	PORTCONTROL_INDEX(16,  5),      // MEM_BHE3
	PORTCONTROL_INDEX(16,  6),      // MEM_SD_CSN
	PORTCONTROL_INDEX(16,  7),      // MEM_M_CSN
	PORTCONTROL_INDEX(16,  8),      // MEM_WRN
	PORTCONTROL_INDEX(16,  9),      // MEM_RDN
	PORTCONTROL_INDEX(16, 10),      // MEM_RDY
	PORTCONTROL_INDEX( 9,  0)       // MEM_SDCLK
};



void setup_hif_and_mem_portctrl(void)
{
	portcontrol_apply(ausPortControlIndex_HifMem, g_t_romloader_options.t_hif_options.ausPortControl, sizeof(ausPortControlIndex_HifMem)/sizeof(ausPortControlIndex_HifMem[0]));
}



static int activate_ddr_clocks(void)
{
	HOSTDEF(ptRAPSysctrlArea);
	TIMER_HANDLE_T tHandle;
	unsigned long ulValue;
	int iElapsed;
	int iResult;


	/* Expect success. */
	iResult = 0;

	/* Is the power already enabled? */
	ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT;
	ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRSTAT_DDR);
	if( ulValue==0 )
	{
		/* No, the power is not yet enabled. Do this now. */

		/* Can the DDR clock be enabled? */
		ulValue   = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRMASK;
		ulValue  &= HOSTMSK(RAP_SYSCTRL_NOCPWRMASK_DDR);
		if( ulValue==0 )
		{
			/* No, the power can not be enabled. */
			//trace_message(TRACEMSG_Memory_DDR_PowerMaskedOut);
			iResult = -1;
		}
		else
		{
			/* Enable the clocks. */
			ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG;
			ulValue |= HOSTMSK(RAP_SYSCTRL_CLKCFG_DDR);
			ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG = ulValue;

			/* Enable the power. */
			ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL;
			ulValue |= HOSTMSK(RAP_SYSCTRL_NOCPWRCTRL_DDR);
			ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL = ulValue;

			/* Wait until the power is enabled. */
			cr7_global_timer_start_ticks(&tHandle, g_t_romloader_options.tDdrOptions.ulPowerUpTimeoutTicks);
			do
			{
				/* Reached the timeout? */
				iElapsed = cr7_global_timer_elapsed(&tHandle);
				if( iElapsed!=0 )
				{
					//trace_message_ul(TRACEMSG_Memory_DdrPowerNotActive, g_t_romloader_options.tDdrOptions.ulPowerUpTimeoutTicks);
					iResult = -1;
					break;
				}
				else
				{
					ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT;
					ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRSTAT_DDR);
				}
			} while( ulValue==0 );
		}
	}

	return iResult;
}



static unsigned long memory_ddr_get_ul32(const unsigned char *pucScript)
{
	unsigned long ulValue;


	ulValue  = ((unsigned long)pucScript[0]);
	ulValue |= ((unsigned long)pucScript[1]) <<  8U;
	ulValue |= ((unsigned long)pucScript[2]) << 16U;
	ulValue |= ((unsigned long)pucScript[3]) << 24U;

	return ulValue;
}



static int memory_ddr_execute_script(const unsigned char *pucScript, unsigned int sizScript)
{
	volatile unsigned long *pulDdrPhy  = (volatile unsigned long*)HOSTADDR(DDR_PHY);
	volatile unsigned long *pulDdrCtrl = (volatile unsigned long*)HOSTADDR(DDR_CTRL);
	const unsigned char *pucCnt;
	const unsigned char *pucEnd;
	unsigned char ucData;
	int iResult;
	DDR_SETUP_COMMAND_T tCommand;
	unsigned int sizLeft;
	unsigned int uiRegisterIndex;
	unsigned long ulMask;
	unsigned long ulData;
	unsigned long ulTimeoutTicks;
	TIMER_HANDLE_T tTimerHandle;
	unsigned long ulValue;
	int iTimerHasElapsed;


	/* An empty script is no error -> start with the OK state. */
	iResult = 0;

	pucCnt = pucScript;
	pucEnd = pucScript + sizScript;
	while(pucCnt<pucEnd)
	{
		/* Get the command. */
		ucData = *(pucCnt++);

		/* Is the command valid? */
		iResult = -1;
		switch(ucData)
		{
		case DDR_SETUP_COMMAND_Nop:
		case DDR_SETUP_COMMAND_WritePhy:
		case DDR_SETUP_COMMAND_WriteCtrl:
		case DDR_SETUP_COMMAND_DelayTicks:
		case DDR_SETUP_COMMAND_PollPhy:
		case DDR_SETUP_COMMAND_PollCtrl:
			iResult = 0;
			break;
		}
		if( iResult!=0 )
		{
			//trace_message_uc(TRACEMSG_Memory_DDR_script_invalid_command, ucData);
		}
		else
		{
			/* Get the number of bytes left in the script. */
			sizLeft = (unsigned int)(pucEnd - pucCnt);

			tCommand = (DDR_SETUP_COMMAND_T)ucData;
			switch( tCommand )
			{
			case DDR_SETUP_COMMAND_Nop:
				//trace_message(TRACEMSG_Memory_DDR_script_command_nop);
				break;


			case DDR_SETUP_COMMAND_WritePhy:
				/* The WritePhy command needs 5 bytes:
				 *  1 register index
				 *  4 value bytes
				 */
				if( sizLeft<5 )
				{
					//trace_message(TRACEMSG_Memory_DDR_script_not_enough_bytes_left);
					iResult = -1;
				}
				else
				{
					//trace_message_data(TRACEMSG_Memory_DDR_script_command_write_phy, pucCnt, 5);

					/* Get the register index. */
					uiRegisterIndex = (unsigned int)(*(pucCnt++));

					/* Get the register data. */
					ulData = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Write the value to the register. */
					pulDdrPhy[uiRegisterIndex] = ulData;
				}
				break;


			case DDR_SETUP_COMMAND_WriteCtrl:
				/* The WriteCtrl command needs 5 bytes:
				 *  1 register index
				 *  4 value bytes
				 */
				if( sizLeft<5 )
				{
					//trace_message(TRACEMSG_Memory_DDR_script_not_enough_bytes_left);
					iResult = -1;
				}
				else
				{
					//trace_message_data(TRACEMSG_Memory_DDR_script_command_write_ctrl, pucCnt, 5);

					/* Get the register index. */
					uiRegisterIndex = (unsigned int)(*(pucCnt++));

					/* Get the register data. */
					ulData = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Write the value to the register. */
					pulDdrCtrl[uiRegisterIndex] = ulData;
				}
				break;


			case DDR_SETUP_COMMAND_DelayTicks:
				/* The DelayTicks command needs 4 bytes:
				 *  4 bytes for the delay in ticks
				 */
				if( sizLeft<4 )
				{
					//trace_message(TRACEMSG_Memory_DDR_script_not_enough_bytes_left);
					iResult = -1;
				}
				else
				{
					//trace_message_data(TRACEMSG_Memory_DDR_script_command_delay, pucCnt, 4);

					/* Get the timeout in ticks. */
					ulTimeoutTicks = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Execute the delay. */
					cr7_global_timer_delay_ticks(ulTimeoutTicks);
				}
				break;


			case DDR_SETUP_COMMAND_PollPhy:
				/* The PollPhy command needs 13 bytes:
				 *  1 register index
				 *  4 bytes mask
				 *  4 bytes data
				 *  4 bytes timeout
				 */
				if( sizLeft<13 )
				{
					//trace_message(TRACEMSG_Memory_DDR_script_not_enough_bytes_left);
					iResult = -1;
				}
				else
				{
					//trace_message_data(TRACEMSG_Memory_DDR_script_command_poll_phy, pucCnt, 13);

					/* Get the register index. */
					uiRegisterIndex = (unsigned int)(*(pucCnt++));

					/* Get the mask. */
					ulMask = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Get the data. */
					ulData = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Get the timeout. */
					ulTimeoutTicks = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Start the timer. */
					cr7_global_timer_start_ticks(&tTimerHandle, ulTimeoutTicks);

					/* Wait until the register has the requested value. */
					do
					{
						/* Get the current masked register data. */
						ulValue  = pulDdrPhy[uiRegisterIndex];
						ulValue &= ulMask;

						/* Did the timer already run out? */
						iTimerHasElapsed = cr7_global_timer_elapsed(&tTimerHandle);
					} while( ulValue!=ulData && iTimerHasElapsed==0);

					if( iTimerHasElapsed!=0 )
					{
						//trace_message(TRACEMSG_Memory_DDR_script_poll_timed_out);
						iResult = -1;
					}
				}
				break;


			case DDR_SETUP_COMMAND_PollCtrl:
				/* The PollCtrl command needs 13 bytes:
				 *  1 register index
				 *  4 bytes mask
				 *  4 bytes data
				 *  4 bytes timeout
				 */
				if( sizLeft<13 )
				{
					//trace_message(TRACEMSG_Memory_DDR_script_not_enough_bytes_left);
					iResult = -1;
				}
				else
				{
					//trace_message_data(TRACEMSG_Memory_DDR_script_command_poll_ctrl, pucCnt, 13);

					/* Get the register index. */
					uiRegisterIndex = (unsigned int)(*(pucCnt++));

					/* Get the mask. */
					ulMask = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Get the data. */
					ulData = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Get the timeout. */
					ulTimeoutTicks = memory_ddr_get_ul32(pucCnt);
					pucCnt += 4U;

					/* Start the timer. */
					cr7_global_timer_start_ticks(&tTimerHandle, ulTimeoutTicks);

					/* Wait until the register has the requested value. */
					do
					{
						/* Get the current masked register data. */
						ulValue  = pulDdrCtrl[uiRegisterIndex];
						ulValue &= ulMask;

						/* Did the timer already run out? */
						iTimerHasElapsed = cr7_global_timer_elapsed(&tTimerHandle);
					} while( ulValue!=ulData && iTimerHasElapsed==0);

					if( iTimerHasElapsed!=0 )
					{
						//trace_message(TRACEMSG_Memory_DDR_script_poll_timed_out);
						iResult = -1;
					}
				}
				break;
			}
		}

		/* Do not continue to process the script after an error. */
		if( iResult!=0 )
		{
			break;
		}
	}

	return iResult;
}



int memory_setup_ddr(void)
{
	int iResult;
	const unsigned char *pucScript;
	unsigned int sizScript;


	/* An empty script is no error. */
	iResult = 0;
	pucScript = g_t_romloader_options.tDdrOptions.aucScript;
	sizScript  = (unsigned int)( *(pucScript++) );
	sizScript |= (unsigned int)((*(pucScript++))<<8);
	if( sizScript!=0 && (sizScript+2)<=sizeof(g_t_romloader_options.tDdrOptions.aucScript) )
	{
		iResult = activate_ddr_clocks();
		if( iResult==0 )
		{
			iResult = memory_ddr_execute_script(pucScript, sizScript);
			if( iResult==0 )
			{
				//trace_message(TRACEMSG_Memory_DDR_setup_done);
			}
		}
	}

	return iResult;
}

