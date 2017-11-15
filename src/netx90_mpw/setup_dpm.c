
#include <string.h>

#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"


#define __IRQ_LOCK__   {__asm__ volatile ("cpsid   i");}
#define __IRQ_UNLOCK__ {__asm__ volatile ("cpsie   i");}


typedef struct STRUCT_HIF_CONFIGURATION
{
	unsigned long   ulHifIoCfg;
//	unsigned long   ulFirmwareIrqMask;
	unsigned long   ulDpmPioCfg1;
	unsigned short  usDpmPioCfg0;
	unsigned char   ucDpmCfg0x0;
	unsigned char   ucDpmAddrCfg;
	unsigned char   ucDpmTimingCfg;
	unsigned char   ucDpmRdyCfg;
	unsigned char   ucDpmMiscCfg;
	unsigned char   ucDpmIoCfgMisc;

//	unsigned char   ucDpmIrqFiqCfg;
//	unsigned char   ucPcieIntCfg;
//	unsigned short ausPortControl[84];
} HIF_CONFIGURATION_T;

static const HIF_CONFIGURATION_T t_hif_options_default =
{
	/* Set the serial DPM mode to 3. */
	.ulHifIoCfg        = DFLT_VAL_NX90_hif_io_cfg | HOSTMSK(hif_io_cfg_sel_dpm_serial_spo) | HOSTMSK(hif_io_cfg_sel_dpm_serial_sph),
	.ulDpmPioCfg1      = 0,
	.usDpmPioCfg0      = DFLT_VAL_NX90_dpm_pio_cfg0,
	.ucDpmCfg0x0       = DFLT_VAL_NX90_dpm_cfg0x0,
	.ucDpmAddrCfg      = 7U << HOSTSRT(dpm_addr_cfg_addr_range),
	.ucDpmTimingCfg    = DFLT_VAL_NX90_dpm_timing_cfg,
	.ucDpmRdyCfg       = HOSTMSK(dpm_rdy_cfg_rdy_pol) | (1U << HOSTSRT(dpm_rdy_cfg_rdy_drv_mode)),
	.ucDpmMiscCfg      = DFLT_VAL_NX90_dpm_misc_cfg,
	.ucDpmIoCfgMisc    = DFLT_VAL_NX90_dpm_io_cfg_misc
};

HIF_CONFIGURATION_T t_hif_options;


static int dpm_deinit_registers(unsigned int uiUnit)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTADEF(DPM) *ptDpmArea;
	HOSTDEF(ptHandshakeComArea);
	int iResult;
	unsigned int sizCnt;


	/* Be pessimistic. */
	iResult = -1;
	if( uiUnit==0 )
	{
		ptDpmArea = ptDpm0ComArea;
		iResult = 0;
	}
	else if( uiUnit==1 )
	{
		ptDpmArea = ptDpm1ComArea;
		iResult = 0;
	}

	if( iResult==0 )
	{
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
		sizCnt = sizeof(ptHandshakeComArea->aulHandshake_hsc_ctrl)/sizeof(unsigned long);
		do
		{
			ptHandshakeComArea->aulHandshake_hsc_ctrl[--sizCnt] = 0;
		} while( sizCnt!=0 );

		/* Disable all handshake IRQs. */
		ptHandshakeComArea->ulHandshake_dpm_irq_raw_clear = 0xffffffff;
		ptHandshakeComArea->ulHandshake_dpm_irq_msk_reset = 0xffffffff;
		ptHandshakeComArea->ulHandshake_arm_irq_raw_clear = 0xffffffff;
		ptHandshakeComArea->ulHandshake_arm_irq_msk_reset = 0xffffffff;
		ptHandshakeComArea->ulHandshake_xpic_irq_raw_clear = 0xffffffff;
		ptHandshakeComArea->ulHandshake_xpic_irq_msk_reset = 0xffffffff;

		sizCnt = sizeof(ptHandshakeComArea->asHandshake_buf_man)/sizeof(ptHandshakeComArea->asHandshake_buf_man[0]);
		do
		{
			--sizCnt;
			ptHandshakeComArea->asHandshake_buf_man[sizCnt].ulCtrl = HOSTDFLT(handshake_buf_man0_ctrl);
			ptHandshakeComArea->asHandshake_buf_man[sizCnt].ulStatus_ctrl_netx = HOSTDFLT(handshake_buf_man0_status_ctrl_netx);
			ptHandshakeComArea->asHandshake_buf_man[sizCnt].ulWin_map = 0;
		} while( sizCnt!=0 );

		/* Reset all IRQ bits. */
		ptDpmArea->aulDpm_irq_host_dirq_mask_reset[0] = 0xffffffff;
		ptDpmArea->aulDpm_irq_host_dirq_mask_reset[1] = 0xffffffff;
		ptDpmArea->aulDpm_irq_host_sirq_mask_reset[0] = 0xffffffff;
		ptDpmArea->aulDpm_irq_host_sirq_mask_reset[1] = 0xffffffff;
		ptDpmArea->ulDpm_firmware_irq_mask = 0;
	}

	return iResult;
}



#define MESSAGE_DPM_PARALLEL "netX90 MPW parallel DPM"
#define MESSAGE_DPM_SERIAL "netX90 MPW serial DPM"


static int init_dpm(unsigned int uiUnit)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTADEF(DPM) * ptDpmArea;
	HOSTDEF(ptHifIoCtrlArea);
	int iResult;
	unsigned long ulValue;
	unsigned long ulNetxAdr;


	/* Be pessimistic. */
	iResult = -1;
	if( uiUnit==0 )
	{
		ptDpmArea = ptDpm0ComArea;
		iResult = 0;
	}
	else if( uiUnit==1 )
	{
		ptDpmArea = ptDpm1ComArea;
		iResult = 0;
	}

	if( iResult==0 )
	{
		/* Can the DPM clocks be enabled? */
		if( (ptAsicCtrlArea->asClock_enable[0].ulMask & HOSTMSK(clock_enable0_dpm))==0 )
		{
			/* No, the clocks can not be enabled. */
			iResult = -1;
		}
		else
		{
			/* Enable the DPM clock. */
			ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
			ulValue |= HOSTMSK(clock_enable0_dpm);
			__IRQ_LOCK__;
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
			ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;
			__IRQ_UNLOCK__;

			/* Disable the DPM for new configuration. */
			iResult = dpm_deinit_registers(uiUnit);
			if( iResult==0 )
			{
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
				ptDpmArea->ulDpm_cfg0x0       = t_hif_options.ucDpmCfg0x0;
				/* Disable the configuration window. */
				ptDpmArea->ulDpm_addr_cfg     = t_hif_options.ucDpmAddrCfg | HOSTMSK(dpm_addr_cfg_cfg_win_addr_cfg);
				ptDpmArea->ulDpm_timing_cfg   = t_hif_options.ucDpmTimingCfg;
				ptDpmArea->ulDpm_rdy_cfg      = t_hif_options.ucDpmRdyCfg;
				ptDpmArea->ulDpm_misc_cfg     = t_hif_options.ucDpmMiscCfg;
				ptDpmArea->ulDpm_io_cfg_misc  = t_hif_options.ucDpmIoCfgMisc;
				ptDpmArea->aulDpm_pio_cfg[0]  = t_hif_options.usDpmPioCfg0;
				ptDpmArea->aulDpm_pio_cfg[1]  = t_hif_options.ulDpmPioCfg1;

				/* Configure the HIF pins */
				__IRQ_LOCK__;
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
				ptHifIoCtrlArea->ulHif_io_cfg = t_hif_options.ulHifIoCfg;
				__IRQ_UNLOCK__;

				iResult = 0;
			}
		}
	}

	return iResult;
}



static int init_pdpm(unsigned int uiBits)
{
	void *pvDPM;
	unsigned int uiDPMUnit;
	unsigned long ulValue;
	int iResult;


	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 65536);
	memcpy(pvDPM, MESSAGE_DPM_PARALLEL, sizeof(MESSAGE_DPM_PARALLEL));

	/* Activate parallel DPM in the HIF IO configuration. */
	memcpy(&t_hif_options, &t_hif_options_default, sizeof(HIF_CONFIGURATION_T));
	ulValue  = t_hif_options.ulHifIoCfg;
	ulValue |= HOSTMSK(hif_io_cfg_sel_hif_dpm);
	ulValue &= ~HOSTMSK(hif_io_cfg_sel_dpm_serial);
	t_hif_options.ulHifIoCfg = ulValue;

	/* Set the interface width to 16 bits if requested. */
	if( uiBits==16 )
	{
		t_hif_options.ucDpmCfg0x0 = 4;
	}

	/* Always use DPM unit 0 for the parallel DPM. */
	uiDPMUnit = 0;
	iResult = init_dpm(uiDPMUnit);

	return iResult;
}



static int init_sdpm(unsigned int uiUnit)
{
	void *pvDPM;
	unsigned long ulValue;
	int iResult;


	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 65536);
	memcpy(pvDPM, MESSAGE_DPM_SERIAL, sizeof(MESSAGE_DPM_SERIAL));

	/* Activate serial DPM in the HIF IO configuration. */
	memcpy(&t_hif_options, &t_hif_options_default, sizeof(HIF_CONFIGURATION_T));
	ulValue  = t_hif_options.ulHifIoCfg;
	ulValue |= HOSTMSK(hif_io_cfg_sel_hif_dpm);
	ulValue |= HOSTMSK(hif_io_cfg_sel_dpm_serial);
	t_hif_options.ulHifIoCfg = ulValue;

	iResult = init_dpm(uiUnit);

	return iResult;
}


/*-------------------------------------------------------------------------*/

/* These values are extracted from the netX90 ROM code. */

typedef enum CONSOLEMODE_ENUM
{
	CONSOLEMODE_Eth_Intphy  = 0,
	CONSOLEMODE_Eth_Extphy  = 1,
	CONSOLEMODE_Eth_LVDS    = 2,
	CONSOLEMODE_DPM_8bit    = 3,
	CONSOLEMODE_DPM_16bit   = 4,
	CONSOLEMODE_SPM1        = 5,
	CONSOLEMODE_SPM0        = 6,
	CONSOLEMODE_UART        = 7
} CONSOLEMODE_T;

#if 0
static CONSOLEMODE_T get_console_mode(void)
{
	unsigned long ulOldValue;
	unsigned long ulValue;
	unsigned long ulConsoleMode;
	CONSOLEMODE_T tConsoleMode;
	HOSTDEF(ptSqiArea);


	/* Set all SQI PIOs to input. */
	ptSqiArea->ulSqi_pio_oe = 0;

	/* Configure all SQI IOs as PIOs. */
	ulOldValue = ptSqiArea->aulSqi_cr[0];
	ulValue = ulOldValue | HOSTMSK(sqi_cr0_sio_cfg);
	ptSqiArea->aulSqi_cr[0] = ulValue;

	/* Get the strapping options. */
	ulValue = ptSqiArea->ulSqi_pio_in;
	ulConsoleMode  = (ulValue & HOSTMSK(sqi_pio_in_mosi)) >>  HOSTSRT(sqi_pio_in_mosi);
	ulConsoleMode |= (ulValue & HOSTMSK(sqi_pio_in_miso)) >> (HOSTSRT(sqi_pio_in_miso)-1);
	ulConsoleMode |= (ulValue & HOSTMSK(sqi_pio_in_sio2)) >> (HOSTSRT(sqi_pio_in_sio2)-2);
	if( ulConsoleMode>7U )
	{
		ulConsoleMode = 7U;
	}
	tConsoleMode = (CONSOLEMODE_T)ulConsoleMode;

	/* Restore the cr0 register. */
	ptSqiArea->aulSqi_cr[0] = ulOldValue;

	return tConsoleMode;
}
#endif
static CONSOLEMODE_T get_console_mode(void)
{
	unsigned long ulValue;
	unsigned long ulConsoleMode;
	CONSOLEMODE_T tConsoleMode;
	HOSTDEF(ptSampleAtPornStatArea);


	/* Get the strapping options. */
	ulValue = ptSampleAtPornStatArea->aulSample_at_porn_stat_in[1];
	ulConsoleMode  = (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_mosi)) >>  HOSTSRT(sample_at_porn_stat_in1_sqi_mosi);
	ulConsoleMode |= (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_miso)) >> (HOSTSRT(sample_at_porn_stat_in1_sqi_miso)-1);
	ulConsoleMode |= (ulValue & HOSTMSK(sample_at_porn_stat_in1_sqi_sio2)) >> (HOSTSRT(sample_at_porn_stat_in1_sqi_sio2)-2);
	if( ulConsoleMode>7U )
	{
		ulConsoleMode = 7U;
	}
	tConsoleMode = (CONSOLEMODE_T)ulConsoleMode;

	return tConsoleMode;
}

typedef struct
{
	/* Version and chip type. */
	unsigned long ulId;
} ROM_ID_T;

extern const ROM_ID_T tRomId;
#define ROMID_netX90_MPW 0x0010a005


void __attribute__ ((section (".init_code"))) start(void);
void start(void)
{
	int iResult;
	CONSOLEMODE_T tConsoleMode;

	/* Be pessimistic. */
	iResult = -1;

	/* Detect if this is really a relaxed chip. */
	if( tRomId.ulId==ROMID_netX90_MPW )
	{
		/* Get the boot mode. */
		tConsoleMode = get_console_mode();
		switch(tConsoleMode)
		{
		case CONSOLEMODE_Eth_Intphy:
		case CONSOLEMODE_Eth_Extphy:
		case CONSOLEMODE_Eth_LVDS:
			/* Nothing to setup. */
			iResult = 0;
			break;

		case CONSOLEMODE_DPM_8bit:
			/* Setup the parallel DPM with 8 bit. */
			iResult = init_pdpm(8);
			break;

		case CONSOLEMODE_DPM_16bit:
			/* Setup the parallel DPM with 16 bit. */
			iResult = init_pdpm(16);
			break;

		case CONSOLEMODE_SPM1:
			/* Setup the serial DPM 1. */
			iResult = init_sdpm(1);
			break;

		case CONSOLEMODE_SPM0:
			/* Setup the serial DPM 0. */
			iResult = init_sdpm(0);
			break;

		case CONSOLEMODE_UART:
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
