
#include <string.h>

#include "boot_common.h"

#if ASIC_TYP==ASIC_TYP_NETX4000
#include "netx4000_full/boot_dpm.h"
#define PACKAGE_SELECTION_4000 0
#define PACKAGE_SELECTION_4100 1

#elif ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
#include "netx4000_relaxed/boot_dpm.h"
#endif


#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"
#include "setup_dpm.h"

/*-------------------------------------------------------------------------*/

/* These values are extracted from the netX4000 ROM code.
 * They define the position of the boot mode pins in the
 * ASR_ID registers.
 */
/**
 * @brief Construct a DPM message.
 * @details Create a message combining the package type and the DPM type.
 * @param pcDest The destination where the message is created
 * @param pcDpmType DPM type string
 *
 */
void set_dpm_message(char* pcDest, const char* pcDpmType)
{
	HOSTDEF(ptRAPSysctrlArea);
	unsigned long ulPackageSelection = ptRAPSysctrlArea->aulRAP_SYSCTRL_OTP_CONFIG_[0] & 1;
	
	
	/*not allowed message inside DPM is
	  -- netX with upper case X,
		-- BOOT,
		-- INIT,
		because this is used by FW for detection. */
	
	if(ulPackageSelection == PACKAGE_SELECTION_4000)
		strcpy(pcDest, "netx4000 ");
	else
		strcpy(pcDest, "netx4100 ");
	strcat(pcDest, pcDpmType);
}

/**
 * @brief
 * @details
 * @param
 * @return
 *
 */
static BOOT_MODE_LED_T get_boot_mode(void)
{
	HOSTDEF(ptAsicCtrlArea);
	unsigned long ulValue;
	BOOT_MODE_LED_T tBootMode;


	/* Get the boot mode field. */
	ulValue   = ptAsicCtrlArea->aulAsr_id[ASRID_DOFF_BootMode];
	ulValue  &= ASRID_MASK_BootMode;
	ulValue >>= ASRID_BOFF_BootMode;
	/* Get the pins section. */
	ulValue  &= MSK_ASRID_BootMode_Pins;
	ulValue >>= SRT_ASRID_BootMode_Pins;

	/* Cast the value to a boot mode. */
	tBootMode = (BOOT_MODE_LED_T)ulValue;

	/* Return the boot mode. */
	return tBootMode;
}

/**
 * @brief
 * @details
 * @param
 * @return
 *
 */
/* Reset all IRQ bits. */
static void clear_idpm_irqs_(HOSTADEF(IDPM) *ptIdpmArea)
{
	//ptIdpmArea->ulIdpm_irq_host_mask_reset = 0xffffffff;
	//ptIdpmArea->ulIdpm_firmware_irq_mask = 0;
	ptIdpmArea->ulIdpm_irq_pci_inta_mask_reset = 0xffffffff;
	ptIdpmArea->ulIdpm_firmware_irq_mask = 0;
}


/**
 * @brief
 * @details
 * @param
 * @return
 *
 */
static void deinit_idpm_mapping_(HOSTADEF(IDPM) *ptIdpmArea)
{
	/* Disable all windows and write protect them. */
	ptIdpmArea->ulIdpm_win1_end = 0U;
	ptIdpmArea->ulIdpm_win1_map = HOSTMSK(idpm_win1_map_wp_cfg_win);
	ptIdpmArea->ulIdpm_win2_end = 0U;
	ptIdpmArea->ulIdpm_win2_map = 0U;
	ptIdpmArea->ulIdpm_win3_end = 0U;
	ptIdpmArea->ulIdpm_win3_map = 0U;
	ptIdpmArea->ulIdpm_win4_end = 0U;
	ptIdpmArea->ulIdpm_win4_map = 0U;

	/* Disable the tunnel and write protect it. */
	ptIdpmArea->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_cfg_win);
}

/**
 * @brief
 * @details
 * @param
 * @return
 *
 */

static void idpm_configure_(HOSTADEF(IDPM) *ptIdpmArea, const IDPM_CONFIGURATION_T* ptIdpmConfig, unsigned int idpm)
{
	unsigned long ulNetxAdr;
	unsigned long ulValue;

	/* DPM mapping:
	 * 0x0000 - 0xffff : intramhs_dpm_mirror
	 */

	ptIdpmArea->ulIdpm_win1_end = 0U; /* 0x7fffU + 1; */

	switch(idpm){
		case 0:
			ulNetxAdr = HOSTADDR(intramhs0_dpm_mirror);
		break;
		case 1:
			ulNetxAdr = HOSTADDR(intramhs1_dpm_mirror);
		break;
	}
	// ulValue  = (ulNetxAdr-0x0100U) & HOSTMSK(idpm_win1_map_win_map);
	// ulValue |= HOSTMSK(idpm_win1_map_wp_cfg_win);
	ptIdpmArea->ulIdpm_win1_map = 0U;

	ptIdpmArea->ulIdpm_win2_end = 0U;
	ptIdpmArea->ulIdpm_win2_map = 0U;

	ptIdpmArea->ulIdpm_win3_end = 0U;
	ptIdpmArea->ulIdpm_win3_map = 0U;

	ptIdpmArea->ulIdpm_win4_end = 0U;
	ptIdpmArea->ulIdpm_win4_map = 0U;

	/*
	ptIdpmArea->ulIdpm_win1_end = ptIdpmConfig->ulIdpm_win1_end;
	ptIdpmArea->ulIdpm_win1_map = ptIdpmConfig->ulIdpm_win1_map;

	ptIdpmArea->ulIdpm_win2_end = ptIdpmConfig->ulIdpm_win2_end;
	ptIdpmArea->ulIdpm_win2_map = ptIdpmConfig->ulIdpm_win2_map;

	ptIdpmArea->ulIdpm_win3_end = ptIdpmConfig->ulIdpm_win3_end;
	ptIdpmArea->ulIdpm_win3_map = ptIdpmConfig->ulIdpm_win3_map;

	ptIdpmArea->ulIdpm_win4_end = ptIdpmConfig->ulIdpm_win4_end;
	ptIdpmArea->ulIdpm_win4_map = ptIdpmConfig->ulIdpm_win4_map;
	 */

	/* Disable the tunnel and write protect it. */
	ptIdpmArea->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_cfg_win);

	/* configure DPM */
	ptIdpmArea->ulIdpm_cfg0x0       = ptIdpmConfig->ulIdpmCfg0x0;
	ptIdpmArea->ulIdpm_addr_cfg       = ptIdpmConfig->ulIdpmAddrCfg;

}


/**
 * @brief
 * @details
 * @param
 * @return
 *
 */
static void init_intramhs(unsigned int idpm)
{
	void *pvDPM;
	switch(idpm){
	case 0:
		pvDPM = (void*)HOSTADDR(intramhs0_straight_mirror);
		memset(pvDPM, 0, MESSAGE_SIZE);
		set_dpm_message((char*) pvDPM, MESSAGE_IDPM);
		break;
		
	case 1:
		pvDPM = (void*)HOSTADDR(intramhs1_straight_mirror);
		memset(pvDPM, 0, MESSAGE_SIZE);
		set_dpm_message((char*) pvDPM, MESSAGE_IDPM);
		break;
	}
}


/**
 * @brief initialize the handshake area
 * @details
 * @param ptHandshakeCtrlArea[in]
 * @return
 */
static void init_handshake_area(NX4000_HANDSHAKE_CTRL_AREA_T* ptHandshakeCtrlArea)
{
	unsigned int sizCnt;

	/* Read all handshake registers and disable them. */
	sizCnt = sizeof(ptHandshakeCtrlArea->aulHandshake_hsc_ctrl)/sizeof(unsigned long);
	do
	{
		ptHandshakeCtrlArea->aulHandshake_hsc_ctrl[--sizCnt] = 0;
	} while( sizCnt!=0 );

	/* Disable all handshake IRQs. */
	ptHandshakeCtrlArea->ulHandshake_dpm_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrlArea->ulHandshake_dpm_irq_msk_reset = 0xffffffff;
	ptHandshakeCtrlArea->ulHandshake_arm_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrlArea->ulHandshake_arm_irq_msk_reset = 0xffffffff;
	ptHandshakeCtrlArea->ulHandshake_xpic_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrlArea->ulHandshake_xpic_irq_msk_reset = 0xffffffff;

	sizCnt = sizeof(ptHandshakeCtrlArea->asHandshake_buf_man)/sizeof(ptHandshakeCtrlArea->asHandshake_buf_man[0]);
	do
	{
		--sizCnt;
		ptHandshakeCtrlArea->asHandshake_buf_man[sizCnt].ulCtrl = HOSTDFLT(handshake_buf_man0_ctrl);
		ptHandshakeCtrlArea->asHandshake_buf_man[sizCnt].ulStatus_ctrl_netx = HOSTDFLT(handshake_buf_man0_status_ctrl_netx);
		ptHandshakeCtrlArea->asHandshake_buf_man[sizCnt].ulWin_map = 0;
	} while( sizCnt!=0 );
}


/**
 * @brief boots an IDPM
 * @details this function boots an IDPM based on the ptIdpmArea
 * @param[in/out] ptIdpmArea points to the address space of the IDPM that has to be booted
 * @param[in] ptIdpmConfig points to the address space that contains the configuration parameter for the IDPM
 * @param[in] idpm is a variable that simply contains the number of the IDPM that is beeing booted => IDPM0 or IDPM1
 * @return iResult
 */
BOOTING_T boot_idpm(HOSTADEF(IDPM) *ptIdpmArea, IDPM_CONFIGURATION_T* ptIdpmConfig, unsigned int idpm);
BOOTING_T boot_idpm(HOSTADEF(IDPM) *ptIdpmArea, IDPM_CONFIGURATION_T* ptIdpmConfig, unsigned int idpm)
{
	HOSTDEF(ptHandshakeCtrl0Area);
	HOSTDEF(ptHandshakeCtrl1Area);

	BOOTING_T iResult = 0;

	///initialize the handshake register
	init_intramhs(idpm);

	/// deinitialize the idpm mapping
	deinit_idpm_mapping_(ptIdpmArea);

	/**
	 * based on the variable idpm either
	 */
	switch(idpm){
	case 0:
		init_handshake_area(ptHandshakeCtrl0Area);
		break;
	case 1:
		init_handshake_area(ptHandshakeCtrl1Area);
		break;
	}


	clear_idpm_irqs_(ptIdpmArea);

	idpm_configure_(ptIdpmArea, ptIdpmConfig, idpm);

	return iResult;
}


/**
 *@brief boots dpm/idpm depending on ptDpmConfigAll
 *@details boots dpm/idpm depending on the hand-over parameter in the address space
 *
 *@param[in] ptDpmConfigAll points to the address space where the hand-over parameter are stored
 *@return iResult
 *@see boot_dpm
 *@see boot_pcie
 *@see boot_idpm
 */
BOOTING_T setup_dpm_all(HIF_CONFIG_T* ptDpmConfigAll);
BOOTING_T setup_dpm_all(HIF_CONFIG_T* ptDpmConfigAll)
{
	BOOTING_T iResult = 0;
	HOSTDEF(ptIdpm0Area);
	HOSTDEF(ptIdpm1Area);
	HOSTDEF(ptHandshakeCtrl0Area);
	HOSTDEF(ptHandshakeCtrl1Area);
	//HOSTDEF(ptHifIoCtrlArea);
	//HOSTDEF(ptAsicCtrlArea);

	/**	check ptDpmConfigAll->ulDPMEnable for boot options of DPM
	 * 	depending on the value of ptDpmConfigAll->ulDPMEnable, boot_dpm() is called with the following parameter:
	 * 		0 => DO NOTHING
	 *		1 => DPM_TRANSPORT_TYPE_Parallel => 0 => parallel DPM is booted
	 *  	2 => DPM_TRANSPORT_TYPE_Serial => 1 => parallel DPM is booted
	 */
	switch(ptDpmConfigAll->ulDPMEnable)
	{
	default:
	case 0:
		/*DO NOTHING*/
		break;
	case 1:
		iResult = boot_dpm(DPM_TRANSPORT_TYPE_Parallel, ptDpmConfigAll);
		ptHandshakeCtrl0Area->ulHandshake_cfg = ptDpmConfigAll->tHandshakeCrl0Cfg;
		break;
	case 2:
		iResult = boot_dpm(DPM_TRANSPORT_TYPE_Serial, ptDpmConfigAll);
		ptHandshakeCtrl0Area->ulHandshake_cfg = ptDpmConfigAll->tHandshakeCrl0Cfg;
		break;
	}




	/** check ptDpmConfigAll->ulIDPM0Enable for boot options of IDPM0
	 *  	0 => Do NOTHING
	 *  	1 => boot PCIe on IDPM0
	 *  	2 => boot IDPM
	 */
	switch(ptDpmConfigAll->ulIDPM0Enable)
	{
	default:
	case 0:
		/*DO NOTHING*/
		break;
	case 1:

		boot_idpm(ptIdpm0Area, &ptDpmConfigAll->tIdpm0Config, IDPM0);
		
		/*boot idpm0 as pcie*/
		iResult = boot_pcie(IDPM0);
		
		ptHandshakeCtrl0Area->ulHandshake_cfg = ptDpmConfigAll->tHandshakeCrl0Cfg;
		break;
	case 2:

		/*boot idpm0 only*/
		boot_idpm(ptIdpm0Area, &ptDpmConfigAll->tIdpm0Config, IDPM0);
		ptHandshakeCtrl0Area->ulHandshake_cfg = ptDpmConfigAll->tHandshakeCrl0Cfg;
		break;
	}

	/** check ptDpmConfigAll->ulIDPM1Enable for boot options of IDPM1
	 *  	0 => Do NOTHING
	 *  	1 => not implemented
	 *  	2 => boot IDPM
	 */
	switch(ptDpmConfigAll->ulIDPM1Enable)
	{
	default:
	case 0:
		/*DO NOTHING*/
		break;
	case 1:
		break;
	case 2:
		ptIdpm1Area->ulIdpm_cfg0x0 |= HOSTMSK(idpm_cfg0x0_enable);
		/*boot idpm1 only*/
		boot_idpm(ptIdpm1Area, &ptDpmConfigAll->tIdpm1Config, IDPM1);
		ptHandshakeCtrl1Area->ulHandshake_cfg = ptDpmConfigAll->tHandshakeCrl1Cfg;
		break;
	}



	return iResult;
}


/**
 *@brief boot dpm based on ptDpmConfig
 *@details this function is a "quick" option for booting the dpm.
 *@details to boot the following options, ptDpmConfig has to have the following value:
 *@details	0 => default behavior => get boot option from external pins
 *@details 	1 => BootModeLed_netX_HIF_DPM_serial
 *@details 	4 => BootModeLed_netX_HIF_DPM_parallel
 *@details 	7 => BootModeLed_REE_DPM_PCIE
 *@details 	the numbers are translated from the hand over parameter (boot parameter) to the firmware
 *
 *@param[in] ptDpmConfig in this function the pointer is used as a hand over parameter itself, it contains the option for the boot mode
 *@return iResult
 */
int setup_boot_mode_dpm(HIF_CONFIG_T* ptDpmConfig);
int setup_boot_mode_dpm(HIF_CONFIG_T* ptDpmConfig)
{
	int iResult;
	BOOT_MODE_LED_T tBootMode;
	BOOTING_T tResult;

	/* Be pessimistic. */
	iResult = -1;

	/* Detect if this is really a FULL chip. */
	if( tRomVector.ulInfo==ROMVECTOR_INFO_netX4000_FULL )
	{
	
        if (ptDpmConfig == (HIF_CONFIG_T*)1)
		{
			tBootMode = BootModeLed_netX_HIF_DPM_serial;
		}
     	else if (ptDpmConfig == (HIF_CONFIG_T*)7)
		{
			tBootMode = BootModeLed_REE_DPM_PCIE;
		}
     	else if (ptDpmConfig == (HIF_CONFIG_T*)4)
		{
			tBootMode = BootModeLed_netX_HIF_DPM_parallel;
		} else 
    	{
    		/* Get the boot mode. */
    		tBootMode = get_boot_mode();
    	} 			

		
		/* Always enable the IDPM - Please remove if handover parameter is added */
		HOSTDEF(ptIdpm0Area);
		ptIdpm0Area->ulIdpm_cfg0x0 |= HOSTMSK(idpm_cfg0x0_enable);

		
		switch(tBootMode)
		{
		case BootModeLed_REE_SD:
			/* Nothing to setup. */
			iResult = 0;
			break;

		case BootModeLed_netX_HIF_DPM_serial:
			/* Setup the serial DPM. */
			tResult = boot_dpm(DPM_TRANSPORT_TYPE_Serial, NULL);
			if( tResult==BOOTING_Ok )
			{
				iResult = 0;
			}
			break;

		case BootModeLed_netX_ETH_LVDS:
		case BootModeLed_minimal_update:
			/* Nothing to setup. */
			iResult = 0;
			break;

		case BootModeLed_REE_DPM_PCIE:
			/* Setup the PCI express DPM. */
			tResult = boot_pcie(IDPM0);
			if( tResult==BOOTING_Ok )
			{
				iResult = 0;
			}
			break;

		case BootModeLed_netX_HIF_DPM_parallel:
			/* Setup the parallel DPM. */
			tResult = boot_dpm(DPM_TRANSPORT_TYPE_Parallel, NULL);
			if( tResult==BOOTING_Ok )
			{
				iResult = 0;
			}
			break;

		case BootModeLed_REE_PL353_NAND:
		case BootModeLed_netX_MEM_ETH:
			/* Nothing to setup. */
			iResult = 0;
			break;
		}
	}

	return iResult;
}


/**
 * @brief start function for dpm setup
 * @details this function either calls setup_boot_mode_dpm() or setup_dpm_all() depending on the value of ptDpmConfig.
 * @details		if ptDpmConfig is greater then '0x04000000' we assume it is actually a pointer to an address space and we call setup_dpm_all()
 * @details			->in this case the board-config-.xml file has to contain the correct hand-over parameter
 * @details		else setup_boot_mode_dpm() is called
 * @details
 * @details		if the called function returns with a result that is not '0' we go directly into an endless loop.
 *
 * @param[in] ptDpmConfig pointer to the location of the hand-over parameter
 * @see setup_boot_mode_dpm
 * @see setup_dpm_all
 *
 */
void __attribute__ ((section (".init_code"))) start(HIF_CONFIG_T* ptDpmConfig);
void start(HIF_CONFIG_T* ptDpmConfig)
{
	int iResult;

	if(ptDpmConfig == NULL || ptDpmConfig < (HIF_CONFIG_T*)0x04000000)
	{
		iResult = setup_boot_mode_dpm(ptDpmConfig);
	}
	else
	{
		iResult = setup_dpm_all(ptDpmConfig);
	}

	if( iResult!=0 )
	{
		rdy_run_setLEDs(RDYRUN_YELLOW);
		while(1) {}
	}
}
