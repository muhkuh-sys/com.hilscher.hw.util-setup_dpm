
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
#define ROMVECTOR_INFO_netX4000_RELAXED 0x00108004


void __attribute__ ((section (".init_code"))) start(void);
void start(void)
{
	int iResult;
	BOOT_MODE_LED_T tBootMode;
	BOOTING_T tResult;

	/* Be pessimistic. */
	iResult = -1;

	/* Detect if this is really a relaxed chip. */
	if( tRomVector.ulInfo==ROMVECTOR_INFO_netX4000_RELAXED )
	{
		/* Get the boot mode. */
		tBootMode = get_boot_mode();
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
