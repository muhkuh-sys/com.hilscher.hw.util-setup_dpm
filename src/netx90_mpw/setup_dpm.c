/* 
Todo:
Currently, a DPM configured via default config gets a different cookie than a DPM configured via config structure.
Should we replace it with a single one?
*/

#include <string.h>

#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"


#define __IRQ_LOCK__   {__asm__ volatile ("cpsid   i");}
#define __IRQ_UNLOCK__ {__asm__ volatile ("cpsie   i");}

typedef struct STRUCT_DPM_CONFIGURATION
{
	unsigned long   ulDpmPioCfg1;
	unsigned short  ulDpmPioCfg0;
	unsigned char   ulDpmCfg0x0;
	unsigned char   ulDpmAddrCfg;
	unsigned char   ulDpmTimingCfg;
	unsigned char   ulDpmRdyCfg;
	unsigned char   ulDpmMiscCfg;
	unsigned char   ulDpmIoCfgMisc;
} DPM_CONFIGURATION_T;

typedef struct 
{
	unsigned char   ulIdpmCfg0x0;  
	unsigned char   ulIdpmAddrCfg; 
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

static const IDPM_CONFIGURATION_T tDefaultIdpmConfig= {
	.ulIdpmCfg0x0 = 0x01,
	.ulIdpmAddrCfg = 030
};

static const DEFAULT_HIF_CONFIG_T t_hif_options_default =
{
	/* Set the serial DPM mode to 3. */
	.ulHifIoCfg        = DFLT_VAL_NX90_hif_io_cfg | HOSTMSK(hif_io_cfg_sel_dpm_serial_spo) | HOSTMSK(hif_io_cfg_sel_dpm_serial_sph),
	.tDpmConfig       = {
		.ulDpmPioCfg1      = 0,
		.ulDpmPioCfg0      = DFLT_VAL_NX90_dpm_pio_cfg0,
		.ulDpmCfg0x0       = DFLT_VAL_NX90_dpm_cfg0x0,
		.ulDpmAddrCfg      = 7U << HOSTSRT(dpm_addr_cfg_addr_range),
		.ulDpmTimingCfg    = DFLT_VAL_NX90_dpm_timing_cfg,
		.ulDpmRdyCfg       = HOSTMSK(dpm_rdy_cfg_rdy_pol) | (1U << HOSTSRT(dpm_rdy_cfg_rdy_drv_mode)),
		.ulDpmMiscCfg      = DFLT_VAL_NX90_dpm_misc_cfg,
		.ulDpmIoCfgMisc    = DFLT_VAL_NX90_dpm_io_cfg_misc
	}
};

DEFAULT_HIF_CONFIG_T t_hif_options;



/* Configure the HIF pins. */
static void set_hif_io_config(unsigned long ulHifIoCfg)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;    /* @suppress("Assignment to itself") */
	ptHifIoCtrlArea->ulHif_io_cfg = ulHifIoCfg;
	__IRQ_UNLOCK__;
}

static int enable_dpm_clock(void)
{
	HOSTDEF(ptAsicCtrlArea);
	int iResult;
	unsigned long ulValue;

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
		iResult = 0;
	}
	
	return iResult;
}



static void deinit_dpm_mapping(HOSTADEF(DPM) *ptDpmArea)
{
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
}

static void deinit_idpm_mapping(HOSTADEF(IDPM) *ptIdpmArea)
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



/* Reset all IRQ bits. */
static void clear_dpm_irqs(HOSTADEF(DPM) *ptDpmArea)
{
	ptDpmArea->aulDpm_irq_host_dirq_mask_reset[0] = 0xffffffff;
	ptDpmArea->aulDpm_irq_host_dirq_mask_reset[1] = 0xffffffff;
	ptDpmArea->aulDpm_irq_host_sirq_mask_reset[0] = 0xffffffff;
	ptDpmArea->aulDpm_irq_host_sirq_mask_reset[1] = 0xffffffff;
	ptDpmArea->ulDpm_firmware_irq_mask = 0;
}


/* Reset all IRQ bits. */
static void clear_idpm_irqs(HOSTADEF(IDPM) *ptIdpmArea)
{
	ptIdpmArea->ulIdpm_irq_host_mask_reset = 0xffffffff;
	ptIdpmArea->ulIdpm_firmware_irq_mask = 0;
}


static void dpm_configure(HOSTADEF(DPM) *ptDpmArea, DPM_CONFIGURATION_T* ptDpmConfig)
{
	unsigned long ulNetxAdr;
	unsigned long ulValue;
	
	/* DPM mapping:
	 * 0x0000 - 0xffff : intramhs_dpm_mirror
	 */
	ptDpmArea->ulDpm_win1_end = 0x8000; /* 0x7fffU + 1; */
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
	ptDpmArea->ulDpm_cfg0x0       = ptDpmConfig->ulDpmCfg0x0;
	ptDpmArea->ulDpm_addr_cfg     = ptDpmConfig->ulDpmAddrCfg;
	ptDpmArea->ulDpm_timing_cfg   = ptDpmConfig->ulDpmTimingCfg;
	ptDpmArea->ulDpm_rdy_cfg      = ptDpmConfig->ulDpmRdyCfg;
	ptDpmArea->ulDpm_misc_cfg     = ptDpmConfig->ulDpmMiscCfg;
	ptDpmArea->ulDpm_io_cfg_misc  = ptDpmConfig->ulDpmIoCfgMisc;
	ptDpmArea->aulDpm_pio_cfg[0]  = ptDpmConfig->ulDpmPioCfg0;
	ptDpmArea->aulDpm_pio_cfg[1]  = ptDpmConfig->ulDpmPioCfg1;
}

static void idpm_configure(HOSTADEF(IDPM) *ptIdpmArea, const IDPM_CONFIGURATION_T* ptIdpmConfig)
{
	unsigned long ulNetxAdr;
	unsigned long ulValue;
	
	/* DPM mapping:
	 * 0x0000 - 0xffff : intramhs_dpm_mirror
	 */
	ptIdpmArea->ulIdpm_win1_end = 0x8000; /* 0x7fffU + 1; */
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


	/* Disable the tunnel and write protect it. */
	ptIdpmArea->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_cfg_win);

	/* configure DPM */
	ptIdpmArea->ulIdpm_cfg0x0       = ptIdpmConfig->ulIdpmCfg0x0;
	ptIdpmArea->ulIdpm_addr_cfg     = ptIdpmConfig->ulIdpmAddrCfg;
}


static void init_handshake_area(void)
{
	HOSTDEF(ptHandshakeComArea);
	unsigned int sizCnt;

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
}



/*-------------------------------------------------------------------------*/

/*
 * disable the interface
 */

static int dpm_deinit_registers(unsigned int uiUnit)
{
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTADEF(DPM) *ptDpmArea;
	int iResult;


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

		/* Configure the HIF pins. */
		set_hif_io_config(HOSTDFLT(hif_io_cfg));

		/* Disable all windows and write protect them. */
		/* Disable the tunnel and write protect it. */
		deinit_dpm_mapping(ptDpmArea);

		init_handshake_area();

		/* Reset all IRQ bits. */
		clear_dpm_irqs(ptDpmArea);
	}

	return iResult;
}



static int init_dpm(unsigned int uiUnit)
{
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTADEF(DPM) * ptDpmArea;
	int iResult;

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
		iResult = enable_dpm_clock();
		
		if (iResult==0)
		{
			/* Disable the DPM for new configuration. */
			iResult = dpm_deinit_registers(uiUnit);
			if( iResult==0 )
			{
				dpm_configure(ptDpmArea, &t_hif_options.tDpmConfig);
				
				/* Configure the HIF pins */
				set_hif_io_config(t_hif_options.ulHifIoCfg);
			}
		}
	}

	return iResult;
}

/***************************************************************
                 Default DPM config
***************************************************************/

#define MESSAGE_DPM_PARALLEL "netX90 MPW parallel DPM"
#define MESSAGE_DPM_SERIAL "netX90 MPW serial DPM"

static int init_default_pdpm(unsigned int uiBits)
{
	void *pvDPM;
	unsigned int uiDPMUnit;
	unsigned long ulValue;
	int iResult;


	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 32768);
	memcpy(pvDPM, MESSAGE_DPM_PARALLEL, sizeof(MESSAGE_DPM_PARALLEL));

	/* Activate parallel DPM in the HIF IO configuration. */
	memcpy(&t_hif_options, &t_hif_options_default, sizeof(DPM_CONFIGURATION_T));
	ulValue  = t_hif_options.ulHifIoCfg;
	ulValue |= HOSTMSK(hif_io_cfg_sel_hif_dpm);
	ulValue &= ~HOSTMSK(hif_io_cfg_sel_dpm_serial);
	t_hif_options.ulHifIoCfg = ulValue;

	/* Disable the configuration window. */
	t_hif_options.tDpmConfig.ulDpmAddrCfg |= HOSTMSK(dpm_addr_cfg_cfg_win_addr_cfg);

	/* Set the interface width to 16 bits if requested. */
	if( uiBits==16 )
	{
		t_hif_options.tDpmConfig.ulDpmCfg0x0 = 4;
	}
	/* Always use DPM unit 0 for the parallel DPM. */
	uiDPMUnit = 0;
	iResult = init_dpm(uiDPMUnit);

	return iResult;
}



static int init_default_sdpm(unsigned int uiUnit)
{
	void *pvDPM;
	unsigned long ulValue;
	int iResult;


	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 32768);
	memcpy(pvDPM, MESSAGE_DPM_SERIAL, sizeof(MESSAGE_DPM_SERIAL));

	/* Activate serial DPM in the HIF IO configuration. */
	memcpy(&t_hif_options, &t_hif_options_default, sizeof(DPM_CONFIGURATION_T));
	ulValue  = t_hif_options.ulHifIoCfg;
	ulValue |= HOSTMSK(hif_io_cfg_sel_hif_dpm);
	ulValue |= HOSTMSK(hif_io_cfg_sel_dpm_serial);
	t_hif_options.ulHifIoCfg = ulValue;

	iResult = init_dpm(uiUnit);

	return iResult;
}




/* These values are extracted from the netX90 ROM code. */

typedef enum CONSOLEMODE_ENUM
{
	CONSOLEMODE_Eth_Intphy  = 7,
	CONSOLEMODE_Eth_Extphy  = 6,
	CONSOLEMODE_Eth_LVDS    = 5,
	CONSOLEMODE_DPM_8bit    = 4,
	CONSOLEMODE_DPM_16bit   = 3,
	CONSOLEMODE_SPM1        = 2,
	CONSOLEMODE_SPM0        = 1,
	CONSOLEMODE_UART        = 0
} CONSOLEMODE_T;

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


static int setup_console_mode_dpm(void)
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
			iResult = init_default_pdpm(8);
			break;

		case CONSOLEMODE_DPM_16bit:
			/* Setup the parallel DPM with 16 bit. */
			iResult = init_default_pdpm(16);
			break;

		case CONSOLEMODE_SPM1:
			/* Setup the serial DPM 1. */
			iResult = init_default_sdpm(1);
			break;

		case CONSOLEMODE_SPM0:
			/* Setup the serial DPM 0. */
			iResult = init_default_sdpm(0);
			break;

		case CONSOLEMODE_UART:
			/* Nothing to setup. */
			iResult = 0;
			break;
		}
	}
	return iResult;
}



/*-------------------------------------------------------------------------*/



/*
1. Check if the DPM clock can be enabled.
   If yes: enable it.
   If no: error
   
2. Clear the intram HS area and put the DPM cookie at the beginning.

3. Set HIF IO CFG to a default value.

4. Initialize DPM0, DPM1 and IDPM, if a configuration is present.

5. Set HIF IO CFG to the configured value.
*/

#define MESSAGE_DPM "netX90 MPW DPM"

int setup_dpm_all(HIF_CONFIG_T* ptDpmConfigAll);
int setup_dpm_all(HIF_CONFIG_T* ptDpmConfigAll)
{
	HOSTDEF(ptDpm0ComArea);
	HOSTDEF(ptDpm1ComArea);
	HOSTDEF(ptIdpmComArea);
	void *pvDPM;
	int iResult;
	
	/* Be pessimistic. */
	iResult = -1;
	
	/* Leave a message in the DPM. */
	pvDPM = (void*)HOSTADDR(intramhs_straight_mirror);
	memset(pvDPM, 0, 32768);
	memcpy(pvDPM, MESSAGE_DPM, sizeof(MESSAGE_DPM));
	
	iResult = enable_dpm_clock();
	if (iResult==0)
	{
		/* Configure the HIF pins to default. */
		set_hif_io_config(HOSTDFLT(hif_io_cfg));
		
		/* Disable all windows and write protect them. */
		/* Disable the tunnel and write protect it. */
		deinit_dpm_mapping(ptDpm0ComArea);
		deinit_dpm_mapping(ptDpm1ComArea);
		deinit_idpm_mapping(ptIdpmComArea);
		
		init_handshake_area();
		
		/* Reset all IRQ bits. */
		clear_dpm_irqs(ptDpm0ComArea);
		clear_dpm_irqs(ptDpm1ComArea);
		clear_idpm_irqs(ptIdpmComArea);
		
		/* Configure all DPM blocks which are enabled in the configuration structure. */
		if (ptDpmConfigAll->ulDPM0Enable != 0)
		{
			dpm_configure(ptDpm1ComArea, &(ptDpmConfigAll->tDpm1Config));
		}
		dpm_configure(ptDpm0ComArea, &(ptDpmConfigAll->tDpm0Config));
		if (ptDpmConfigAll->ulDPM1Enable != 0)
		{
			dpm_configure(ptDpm1ComArea, &(ptDpmConfigAll->tDpm1Config));
		}
		if (ptDpmConfigAll->ulIDPMEnable != 0)
		{
			idpm_configure(ptIdpmComArea, &(ptDpmConfigAll->tIdpmConfig));
		}
		
		/* If no DPM is configured, enable the internal DPM. */
		if ((ptDpmConfigAll->ulDPM0Enable == 0) &&
			(ptDpmConfigAll->ulDPM1Enable == 0) &&
			(ptDpmConfigAll->ulIDPMEnable == 0))
			{
				idpm_configure(ptIdpmComArea, &tDefaultIdpmConfig);
			}
		
		/* Configure the HIF pins */
		set_hif_io_config(ptDpmConfigAll->ulHifIoCfg);
	}
	
	return iResult;
}



/*-------------------------------------------------------------------------*/


void __attribute__ ((section (".init_code"))) start(HIF_CONFIG_T* ptDpmConfig);
void start(HIF_CONFIG_T* ptDpmConfig)
{
	int iResult;

	if (ptDpmConfig == NULL)
	{
		iResult = setup_console_mode_dpm();
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
