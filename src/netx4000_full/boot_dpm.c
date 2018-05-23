/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include <string.h>

#include "boot_dpm.h"

#include "options.h"


#define MESSAGE_DPM_SERIAL   "netX4000 RELAXED serial DPM"
#define MESSAGE_DPM_PARALLEL "netX4000 RELAXED parallel DPM"
#define MESSAGE_DPM_PCIE     "netX4000 RELAXED PCI express DPM"


/*---------------------------------------------------------------------------*/


static void dpm_deinit_registers(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	HOSTDEF(ptDpmArea);
	HOSTDEF(ptHandshakeCtrl0Area);
	unsigned int sizCnt;


	/*
	 * disable the interface
	 */

	/* Configure the HIF pins. */
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
	ptHifIoCtrlArea->ulHif_io_cfg = HOSTDFLT(hif_io_cfg);
	__IRQ_UNLOCK__;

	/* Disable all windows and write protect them. */
	ptDpmArea->ulDpm_win1_end = 0U;
	ptDpmArea->ulDpm_win1_map = HOSTMSK(dpm_win1_map_wp_cfg_win);
	ptDpmArea->ulDpm_win2_end = 0U;
	ptDpmArea->ulDpm_win2_map = 0U;
	ptDpmArea->ulDpm_win3_end = 0U;
	ptDpmArea->ulDpm_win3_map = 0U;
	ptDpmArea->ulDpm_win4_end = 0U;
	ptDpmArea->ulDpm_win4_map = 0U;

	/* Disable the tunnel and write protect it. */
	ptDpmArea->ulDpm_tunnel_cfg = HOSTMSK(dpm_tunnel_cfg_wp_cfg_win);

	/* Read all handshake registers and disable them. */
	sizCnt = sizeof(ptHandshakeCtrl0Area->aulHandshake_hsc_ctrl)/sizeof(unsigned long);
	do
	{
		ptHandshakeCtrl0Area->aulHandshake_hsc_ctrl[--sizCnt] = 0;
	} while( sizCnt!=0 );

	/* Disable all handshake IRQs. */
	ptHandshakeCtrl0Area->ulHandshake_dpm_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrl0Area->ulHandshake_dpm_irq_msk_reset = 0xffffffff;
	ptHandshakeCtrl0Area->ulHandshake_arm_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrl0Area->ulHandshake_arm_irq_msk_reset = 0xffffffff;
	ptHandshakeCtrl0Area->ulHandshake_xpic_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrl0Area->ulHandshake_xpic_irq_msk_reset = 0xffffffff;

	sizCnt = sizeof(ptHandshakeCtrl0Area->asHandshake_buf_man)/sizeof(ptHandshakeCtrl0Area->asHandshake_buf_man[0]);
	do
	{
		--sizCnt;
		ptHandshakeCtrl0Area->asHandshake_buf_man[sizCnt].ulCtrl = HOSTDFLT(handshake_buf_man0_ctrl);
		ptHandshakeCtrl0Area->asHandshake_buf_man[sizCnt].ulStatus_ctrl_netx = HOSTDFLT(handshake_buf_man0_status_ctrl_netx);
		ptHandshakeCtrl0Area->asHandshake_buf_man[sizCnt].ulWin_map = 0;
	} while( sizCnt!=0 );

	/* Reset all IRQ bits. */
	ptDpmArea->ulDpm_irq_arm_mask_reset = 0xffffffff;
	ptDpmArea->ulDpm_irq_xpic_mask_reset = 0xffffffff;
	ptDpmArea->ulDpm_irq_fiq_mask_reset = 0xffffffff;
	ptDpmArea->ulDpm_irq_irq_mask_reset = 0xffffffff;
	ptDpmArea->ulDpm_firmware_irq_mask = 0;
}


/*---------------------------------------------------------------------------*/


static void dpm_init(DPM_TRANSPORT_TYPE_T tDpmTransportType)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptDpmArea);
	HOSTDEF(ptHifIoCtrlArea);
	unsigned long ulValue;
	unsigned long ulNetxAdr;
	void *pvDPM;


	/* Enable the DPM clock. */
	ulValue  = ptAsicCtrlArea->ulClock_enable;
	ulValue |= HOSTMSK(clock_enable_dpm);
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
	ptAsicCtrlArea->ulClock_enable = ulValue;
	__IRQ_UNLOCK__;

	/* Disable the DPM for new configuration. */
	dpm_deinit_registers();

	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 65536);
	switch(tDpmTransportType)
	{
	case DPM_TRANSPORT_TYPE_Parallel:
		memcpy(pvDPM, MESSAGE_DPM_PARALLEL, sizeof(MESSAGE_DPM_PARALLEL));
		break;

	case DPM_TRANSPORT_TYPE_Serial:
		memcpy(pvDPM, MESSAGE_DPM_SERIAL, sizeof(MESSAGE_DPM_SERIAL));
		break;
	}

	/* DPM mapping:
	 * 0x0000 - 0xffff : intramhs_dpm_mirror
	 */
	ptDpmArea->ulDpm_win1_end = 0; /* 0xffffU + 1; */
	ulNetxAdr = HOSTADDR(intramhs_dpm_mirror);
	ulValue  = (ulNetxAdr-0x0100U) & HOSTMSK(dpm_win1_map_win_map);
	ulValue |= ulNetxAdr & HOSTMSK(dpm_win1_map_win_page);
	ulValue |= HOSTMSK(dpm_win1_map_wp_cfg_win);
	ulValue |= HOSTMSK(dpm_win1_map_read_ahead);
	ulValue |= HOSTMSK(dpm_win1_map_byte_area);
	ptDpmArea->ulDpm_win1_map = ulValue;

	ptDpmArea->ulDpm_win2_end = 0U;
	ptDpmArea->ulDpm_win2_map = 0U;

	ptDpmArea->ulDpm_win3_end = 0U;
	ptDpmArea->ulDpm_win3_map = 0U;

	ptDpmArea->ulDpm_win4_end = 0U;
	ptDpmArea->ulDpm_win4_map = 0U;


	/* Disable the tunnel and write protect it. */
	ptDpmArea->ulDpm_tunnel_cfg = HOSTMSK(dpm_tunnel_cfg_wp_cfg_win);


	/* configure DPM */
	ptDpmArea->ulDpm_cfg0x0       = g_t_romloader_options.t_hif_options.ucDpmCfg0x0;
	/* Disable the configuration window. */
	ptDpmArea->ulDpm_addr_cfg     = g_t_romloader_options.t_hif_options.ucDpmAddrCfg | HOSTMSK(dpm_addr_cfg_cfg_win_addr_cfg);
	ptDpmArea->ulDpm_timing_cfg   = g_t_romloader_options.t_hif_options.ucDpmTimingCfg;
	ptDpmArea->ulDpm_rdy_cfg      = g_t_romloader_options.t_hif_options.ucDpmRdyCfg;
	ptDpmArea->ulDpm_misc_cfg     = g_t_romloader_options.t_hif_options.ucDpmMiscCfg;
	ptDpmArea->ulDpm_io_cfg_misc  = g_t_romloader_options.t_hif_options.ucDpmIoCfgMisc;

	/* configure FIQ */
	ulValue = 0;
	if( (g_t_romloader_options.t_hif_options.ucDpmIrqFiqCfg&DPM_IRQFIQ_CFG_Fiq_DPM_Sw)!=0 )
	{
		ulValue |= HOSTMSK(dpm_irq_fiq_mask_set_dpm_sw);
	}
	if( (g_t_romloader_options.t_hif_options.ucDpmIrqFiqCfg&DPM_IRQFIQ_CFG_Fiq_DPM_Err)!=0 )
	{
		ulValue |= HOSTMSK(dpm_irq_fiq_mask_set_dpm_err);
	}
	if( (g_t_romloader_options.t_hif_options.ucDpmIrqFiqCfg&DPM_IRQFIQ_CFG_Fiq_Firmware)!=0 )
	{
		ulValue |= HOSTMSK(dpm_irq_fiq_mask_set_firmware);
	}
	ptDpmArea->ulDpm_irq_fiq_mask_set = ulValue;

	/* configure IRQ */
	ulValue = 0;
	if( (g_t_romloader_options.t_hif_options.ucDpmIrqFiqCfg&DPM_IRQFIQ_CFG_Irq_DPM_Sw)!=0 )
	{
		ulValue |= HOSTMSK(dpm_irq_irq_mask_set_dpm_sw);
	}
	if( (g_t_romloader_options.t_hif_options.ucDpmIrqFiqCfg&DPM_IRQFIQ_CFG_Irq_DPM_Err)!=0 )
	{
		ulValue |= HOSTMSK(dpm_irq_irq_mask_set_dpm_err);
	}
	if( (g_t_romloader_options.t_hif_options.ucDpmIrqFiqCfg&DPM_IRQFIQ_CFG_Irq_Firmware)!=0 )
	{
		ulValue |= HOSTMSK(dpm_irq_irq_mask_set_firmware);
	}
	ptDpmArea->ulDpm_irq_irq_mask_set = ulValue;

	/* Configure the firmware IRQ. */
	ptDpmArea->ulDpm_firmware_irq_mask = g_t_romloader_options.t_hif_options.ulFirmwareIrqMask;

	/* Configure the HIF pins */
	ulValue  = g_t_romloader_options.t_hif_options.ulHifIoCfg;
	ulValue |= HOSTMSK(hif_io_cfg_sel_hif_dpm);
	switch(tDpmTransportType)
	{
	case DPM_TRANSPORT_TYPE_Parallel:
		/* Clear the serial DPM bit. */
		ulValue &= ~HOSTMSK(hif_io_cfg_sel_dpm_serial);
		break;

	case DPM_TRANSPORT_TYPE_Serial:
		/* Set the serial DPM bit. */
		ulValue |=  HOSTMSK(hif_io_cfg_sel_dpm_serial);
		break;
	}
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
	ptHifIoCtrlArea->ulHif_io_cfg = ulValue;
	__IRQ_UNLOCK__;

	/* Set the port control configuration for all HIF and MEM pins. */
	setup_hif_and_mem_portctrl();
}



static BOOTING_T pcie_init(void)
{
	HOSTDEF(ptRAPSysctrlArea);
	HOSTDEF(ptPCIEArea);
	BOOTING_T tResult;
	unsigned long ulResetSource;
	unsigned long ulValue;
	TIMER_HANDLE_T tHandle;
	int iElapsed;


	/* Expect success. */
	tResult = BOOTING_Ok;

	/* Test if the PCIe core is already enabled.
	 * This is the case if...
	 *  1) the reset source is "software"
	 *  2) the clock configuration is already enabled.
	 */
	ulResetSource  = ptRAPSysctrlArea->ulRAP_SYSCTRL_RSTSTAT;
	ulResetSource &= HOSTMSK(RAP_SYSCTRL_RSTSTAT_SWRST_STAT) | HOSTMSK(RAP_SYSCTRL_RSTSTAT_STAT_NETX_DPM);
	ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG;
	ulValue &= HOSTMSK(RAP_SYSCTRL_CLKCFG_PCIe);
	if( ulResetSource!=0 && ulValue!=0 )
	{
		/* The PCIe core was already enabled. Only activate the NOC clock manager again. */
		/* Enable the power. */
		ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL;
		ulValue |= HOSTMSK(RAP_SYSCTRL_NOCPWRCTRL_PCIe);
		ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL = ulValue;

		/* Wait until the power is enabled. */
		cr7_global_timer_start_ticks(&tHandle, g_t_romloader_options.tPcieOptions.ulPowerUpTimeoutTicks);
		do
		{
			/* Reached the timeout? */
			iElapsed = cr7_global_timer_elapsed(&tHandle);
			if( iElapsed!=0 )
			{
				//trace_message_ul(TRACEMSG_DPM_PciePowerNotActive, g_t_romloader_options.tPcieOptions.ulPowerUpTimeoutTicks);
				tResult = BOOTING_Setup_Error;
				break;
			}
			else
			{
				ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT;
				ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRSTAT_PCIe);
			}
		} while( ulValue==0 );
	}
	else
	{
		/* The PCIe core was not enabled yet. */

		/* RAP_SYSCTRL_PcieSetup(0, true); */
		ulValue  = HOSTMSK(RAP_SYSCTRL_PCIECFG_MODE_CISRMSEL);       /* input DC-coupling like demanded in base spec */
		ulValue |= HOSTMSK(RAP_SYSCTRL_PCIECFG_MODE_RISRCREN);       /* receiver termination 50 Ohms to GND */
		ulValue |= HOSTMSK(RAP_SYSCTRL_PCIECFG_MODE_TX_DRV_EN);      /* enable output driver */
		ulValue |= HOSTMSK(RAP_SYSCTRL_PCIECFG_MODE_CISRREN);        /* use internal reference clock input termination */
		ulValue |= HOSTMSK(RAP_SYSCTRL_PCIECFG_D3_EVENT_ACK);        /* enable power state D3hot for PCIe endpoint */
		ulValue |= HOSTMSK(RAP_SYSCTRL_PCIECFG_TURN_OFF_EVENT_ACK);  /* ... like D3_EVENT_ACK */
		ptRAPSysctrlArea->ulRAP_SYSCTRL_PCIECFG = ulValue;

		/* Setup the ACP cache policy. */
		ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCAXICFG;
		ulValue &= ~HOSTMSK(RAP_SYSCTRL_NOCAXICFG_PCIE);
		ulValue |= ((unsigned long)(g_t_romloader_options.tPcieOptions.ucAcpCachePolicyPcie)) << HOSTSRT(RAP_SYSCTRL_NOCAXICFG_PCIE);
		ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCAXICFG = ulValue;

		/* Is the power already enabled? */
		ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT;
		ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRSTAT_PCIe);
		if( ulValue==0 )
		{
			/* Can the PCIe power be enabled? */
			ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRMASK;
			ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRMASK_PCIe);
			if( ulValue==0 )
			{
				/* No, it can not be enabled. */
				//trace_message(TRACEMSG_DPM_PciePowerMaskedOut);
				tResult = BOOTING_Not_Allowed;
			}
			else
			{
				/* Yes, the power can be enabled. */
				ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG;
				ulValue |= HOSTMSK(RAP_SYSCTRL_CLKCFG_PCIe);
				ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG = ulValue;

				/* Enable the power. */
				ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL;
				ulValue |= HOSTMSK(RAP_SYSCTRL_NOCPWRCTRL_PCIe);
				ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL = ulValue;

				/* Wait until the power is enabled. */
				cr7_global_timer_start_ticks(&tHandle, g_t_romloader_options.tPcieOptions.ulPowerUpTimeoutTicks);
				do
				{
					/* Reached the timeout? */
					iElapsed = cr7_global_timer_elapsed(&tHandle);
					if( iElapsed!=0 )
					{
						//trace_message_ul(TRACEMSG_DPM_PciePowerNotActive, g_t_romloader_options.tPcieOptions.ulPowerUpTimeoutTicks);
						tResult = BOOTING_Setup_Error;
						break;
					}
					else
					{
						ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT;
						ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRSTAT_PCIe);
					}
				} while( ulValue==0 );
			}
		}

		if( tResult==BOOTING_Ok )
		{
			/* Now the power is up. */

			/* Reset all components. */
			ptPCIEArea->ulPCIE_SW_RST = 0;

			/* Note: The specification says we have to wait 5 milliseconds between clock enable and reset release! */
			cr7_global_timer_delay_ticks(g_t_romloader_options.tPcieOptions.ulResetDelayTicks);

			/* enable configuration registers only */
			ulValue  = HOSTMSK(PCIE_SW_RST_RST_CFG_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_LOAD_B);
			ptPCIEArea->ulPCIE_SW_RST = ulValue;

			/* Set the device ID. */
			ptPCIEArea->ulPCIE_PCI_CR_ID = g_t_romloader_options.tPcieOptions.ulVidPid;
			ptPCIEArea->ulPCIE_PCI_CR_CLASS_REV = g_t_romloader_options.tPcieOptions.ulRevisionClassCode;

			/* The six Base Address Registers have the following functions:
			 * - BAR0 and BAR1 span a 64bit user defined memory space.
			 *   The four AXI (RX) windows are accessible through this memory space.
			 *   Set the required size of the memory space by writing the BAR Mask0 register.
			 * - BAR1 and BAR2 define a 64bit register space for the PCIE registers.
			 *   Registers of the PCI configuration space are not accessible through this window.
			 *   The window size is fixed to 4K. Default BAR Mask1 values should not be changed.
			 * - BAR2 and BAR3 are reserved an can not be used.
			 */
			/* FIXME: the register definition states that the following registers are read only?!?
			 * But this is also the case for the ID and CLASS_REV registers.
			 */
			ptPCIEArea->asPCIE_PCI_CR_BAR_MSK0[0].ulL = 0x00010000 - 1;  /* The window has 64k. */
			/* set maximum supported data size per request */
			ptPCIEArea->ulPCIE_PCI_CR_BASE_SIZE00 = 0x100;

			/* Disable all windows. */
			ptPCIEArea->ulPCIE_AWBASE0 = 0;
			ptPCIEArea->ulPCIE_AWBASE1 = 0;
			ptPCIEArea->ulPCIE_AWBASE2 = 0;
			ptPCIEArea->ulPCIE_AWBASE3 = 0;

			/* Setup and enable the window. */
			ptPCIEArea->ulPCIE_AWMASK0 = 0x00010000 - 1;  /* The window has 64k. */
			ptPCIEArea->ulPCIE_ADEST0  = HOSTADDR(intramhs_dpm_mirror);
			ptPCIEArea->ulPCIE_AWBASE0 = 0 | HOSTMSK(PCIE_AWBASE0_WE);  /* Set the map offset to 0 and enable the window. */

			/* Enable all components. */
			ulValue  = HOSTMSK(PCIE_SW_RST_RST_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_GP_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_RSM_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_CFG_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_LOAD_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_PS_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_OUT_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_PREG_B);
			ptPCIEArea->ulPCIE_SW_RST = ulValue;

			//trace_message(TRACEMSG_DPM_PcieWaitingForLinkUp);
			do
			{
				ulValue  = ptPCIEArea->ulPCIE_PCI_CORE_STATUS_1;
				ulValue &= HOSTMSK(PCIE_PCI_CORE_STATUS_1_DL_DOWN_STATUS);
			} while( ulValue!=0 );
			//trace_message(TRACEMSG_DPM_PcieLinkIsUp);
		}
	}

	return tResult;
}



/*-------------------------------------------------------------------------*/

BOOTING_T boot_dpm(DPM_TRANSPORT_TYPE_T tDpmTransportType)
{
	HOSTDEF(ptAsicCtrlArea);
	BOOTING_T tResult;


	/* Can the DPM clocks be enabled? */
	if( (ptAsicCtrlArea->ulClock_enable_mask&HOSTMSK(clock_enable_dpm))==0 )
	{
		/* No, the clocks can not be enabled. */
//		trace_message(TRACEMSG_DPM_DpmClocksMaskedOut);
		tResult = BOOTING_Not_Allowed;
	}
	else
	{
		/* Initialize the DPM. */
		dpm_init(tDpmTransportType);
		tResult = BOOTING_Ok;
	}

	return tResult;
}



BOOTING_T boot_pcie(void)
{
	HOSTDEF(ptIdpm0Area);
	BOOTING_T tResult;
	void *pvDPM;


	/* NOTE: The PCI express signals are dedicated pins. There is no need
	 *       to check if booting from this source is allowed.
	 */

	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 65536);
	memcpy(pvDPM, MESSAGE_DPM_PCIE, sizeof(MESSAGE_DPM_PCIE));

	/* Setup the PCIE core. */
	tResult = pcie_init();
	if( tResult==BOOTING_Ok )
	{
		ptIdpm0Area->ulIdpm_irq_pci_inta_mask_set = HOSTMSK(idpm_irq_pci_inta_mask_set_firmware);
		
		/* Enable the IDPM */
		ptIdpm0Area->ulIdpm_cfg0x0 |= HOSTMSK(idpm_cfg0x0_enable);		
	}

	return tResult;
}


/*-----------------------------------*/
