
#include <string.h>

#include "boot_common.h"
#include "boot_dpm.h"
#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"

/*-------------------------------------------------------------------------*/

/* These values are extracted from the netX4000 ROM code.
 * They define the position of the boot mode pins in the
 * ASR_ID registers.
 */
#define ASRID_MASK_BootMode 0x000000ffU
#define ASRID_BOFF_BootMode 0
#define ASRID_DOFF_BootMode 1
#define ASRID_BSIZ_BootMode 8

#define MSK_ASRID_BootMode_Pins        0x0000001cU
#define SRT_ASRID_BootMode_Pins        2


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



typedef struct
{
	/* This is the vector table at the start of the ROM. */
	unsigned long aulVectorTable[8];
	/* Version and chip type. */
	unsigned long ulInfo;
} ROMVECTOR_T;

extern ROMVECTOR_T tRomVector;
#define ROMVECTOR_INFO_netX4000_FULL 0x0010b004



typedef struct STRUCT_DPM_CONFIGURATION
{
	unsigned long ulDpmCfg0x0;
	unsigned long ulDpmIfCfg;
	unsigned long ulDpmPioCfg0;
	unsigned long ulDpmPioCfg1;
	unsigned long ulDpmAddrCfg;
	unsigned long ulDpmTimingCfg;
	unsigned long ulDpmRdyCfg;
	unsigned long ulDpmMiscCfg;
	unsigned long ulDpmIoCfgMisc;
} DPM_CONFIGURATION_T;

typedef struct 
{
	unsigned long ulIdpmCfg0x0;  
	unsigned long ulIdpmAddrCfg; 
} IDPM_CONFIGURATION_T;


typedef struct
{
	unsigned long ulHifIoCfg;
	unsigned long ulDPM0Enable;
	DPM_CONFIGURATION_T tDpm0Config;
	unsigned long ulDPM1Enable;
	DPM_CONFIGURATION_T tDpm1Config;
	unsigned long ulIDPMEnable;
	IDPM_CONFIGURATION_T tIdpmConfig;
} HIF_CONFIG_T;

typedef struct
{
	unsigned long ulHifIoCfg;
	DPM_CONFIGURATION_T tDpmConfig;
} DEFAULT_HIF_CONFIG_T;







void __attribute__ ((section (".init_code"))) start(HIF_CONFIG_T* ptDpmConfig);
void start(HIF_CONFIG_T* ptDpmConfig)
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
		
		switch(tBootMode)
		{
		case BootModeLed_REE_SD:
			/* Nothing to setup. */
			iResult = 0;
			break;

		case BootModeLed_netX_HIF_DPM_serial:
			/* Setup the serial DPM. */
			tResult = boot_dpm(DPM_TRANSPORT_TYPE_Serial);
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
			tResult = boot_pcie();
			if( tResult==BOOTING_Ok )
			{
				iResult = 0;
			}
			break;

		case BootModeLed_netX_HIF_DPM_parallel:
			/* Setup the parallel DPM. */
			tResult = boot_dpm(DPM_TRANSPORT_TYPE_Parallel);
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

	if( iResult!=0 )
	{
		rdy_run_setLEDs(RDYRUN_YELLOW);
		while(1) {}
	}
}
