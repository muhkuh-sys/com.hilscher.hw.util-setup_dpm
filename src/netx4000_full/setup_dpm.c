
#include <string.h>

#include "boot_common.h"

#if ASIC_TYP==ASIC_TYP_NETX4000_FULL
#include "netx4000_full/boot_dpm.h"
#define MESSAGE_DPM_SERIAL   "netX4000 serial DPM"
#define MESSAGE_DPM_PARALLEL "netX4000 parallel DPM"
#define MESSAGE_DPM_PCIE     "netX4000 PCI express DPM"
#elif ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
#include "netx4000_relaxed/boot_dpm.h"
#define MESSAGE_DPM_SERIAL   "netX4000 RELAXED serial DPM"
#define MESSAGE_DPM_PARALLEL "netX4000 RELAXED parallel DPM"
#define MESSAGE_DPM_PCIE     "netX4000 RELAXED PCI express DPM"
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

/* Reset all IRQ bits. */
static void clear_idpm_irqs_(HOSTADEF(IDPM) *ptIdpmArea)
{
	//ptIdpmArea->ulIdpm_irq_host_mask_reset = 0xffffffff;
	//ptIdpmArea->ulIdpm_firmware_irq_mask = 0;
	ptIdpmArea->ulIdpm_irq_pci_inta_mask_reset = 0xffffffff;
	ptIdpmArea->ulIdpm_firmware_irq_mask = 0;
}

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

static void idpm_configure_(HOSTADEF(IDPM) *ptIdpmArea, const IDPM_CONFIGURATION_T* ptIdpmConfig)
{
	unsigned long ulNetxAdr;
	unsigned long ulValue;

	/* DPM mapping:
	 * 0x0000 - 0xffff : intramhs_dpm_mirror
	 */

	ptIdpmArea->ulIdpm_win1_end = 0U; /* 0x7fffU + 1; */
	ulNetxAdr = HOSTADDR(intramhs_dpm_mirror);
	ulValue  = (ulNetxAdr-0x0100U) & HOSTMSK(idpm_win1_map_win_map);
	ulValue |= HOSTMSK(idpm_win1_map_wp_cfg_win);
	ptIdpmArea->ulIdpm_win1_map = ulValue;

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
	ptIdpmArea->ulIdpm_addr_cfg     = ptIdpmConfig->ulIdpmAddrCfg;


}

BOOTING_T boot_idpm(HOSTADEF(IDPM) *ptIdpmArea, IDPM_CONFIGURATION_T* ptIdpmConfig);
BOOTING_T boot_idpm(HOSTADEF(IDPM) *ptIdpmArea, IDPM_CONFIGURATION_T* ptIdpmConfig)
{
	BOOTING_T iResult = 0;

	deinit_idpm_mapping_(ptIdpmArea);

	clear_idpm_irqs_(ptIdpmArea);

	idpm_configure_(ptIdpmArea,ptIdpmConfig);

	return iResult;
}







BOOTING_T setup_dpm_all(HIF_CONFIG_T* ptDpmConfigAll);
BOOTING_T setup_dpm_all(HIF_CONFIG_T* ptDpmConfigAll)
{
	BOOTING_T iResult = 0;
	HOSTDEF(ptIdpm0Area);
	HOSTDEF(ptIdpm1Area);

	/*check ulDPMEnable for boot options of DPM*/
	switch(ptDpmConfigAll->ulDPMEnable)
	{
	default:
	case 0:
		/*DO NOTHING*/
		break;
	case 1:
		iResult = boot_dpm(DPM_TRANSPORT_TYPE_Parallel, ptDpmConfigAll);
		break;
	case 2:
		iResult = boot_dpm(DPM_TRANSPORT_TYPE_Serial, ptDpmConfigAll);
		break;
	}

	/*check ulIDPM0Enable for boot options of IDPM0*/
	switch(ptDpmConfigAll->ulIDPM0Enable)
	{
	default:
	case 0:
		/*DO NOTHING*/
		break;
	case 1:
		/*boot idpm0 as pcie*/
		iResult = boot_pcie(IDPM0);
		break;
	case 2:
		/*TO DO*/
		/*boot idpm0 only*/
		boot_idpm(ptIdpm0Area, &ptDpmConfigAll->tIdpm0Config);
		break;
	}

	/*check ulIDPM0Enable for boot options of IDPM0*/
	switch(ptDpmConfigAll->ulIDPM1Enable)
	{
	default:
	case 0:
		/*DO NOTHING*/
		break;
	case 1:
		/*TO DO*/
		/*boot idpm1 as pcie*/
		iResult = boot_pcie(IDPM1);
		break;
	case 2:
		/*TO DO*/
		/*boot idpm1 only*/
		boot_idpm(ptIdpm1Area, &ptDpmConfigAll->tIdpm1Config);
		break;
	}

	return iResult;
}



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

    /* check first parameter (pointer) for following options */
	/* 0 => default behaviour => get boot option from external pins  */
	/* the numbers are translated from the hand over parameter (boot parameter) to the firmware */
    /* 1 =>	BootModeLed_netX_HIF_DPM_serial    => 1                  */
	/* 7 => BootModeLed_REE_DPM_PCIE           => 4                  */
	/* 4 => BootModeLed_netX_HIF_DPM_parallel  => 5                  */
	
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

		HOSTDEF(ptIdpm1Area);
		ptIdpm1Area->ulIdpm_cfg0x0 |= HOSTMSK(idpm_cfg0x0_enable);
		
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
