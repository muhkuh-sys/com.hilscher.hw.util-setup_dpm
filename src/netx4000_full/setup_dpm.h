#ifndef __SETUP_DPM_H__
#define __SETUP_DPM_H__


#define ASRID_MASK_BootMode 0x000000ffU
#define ASRID_BOFF_BootMode 0
#define ASRID_DOFF_BootMode 1
#define ASRID_BSIZ_BootMode 8

#define MSK_ASRID_BootMode_Pins        0x0000001cU
#define SRT_ASRID_BootMode_Pins        2

#define IDPM0 0
#define IDPM1 1

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


	 unsigned long ulDpm_win1_end;
	 unsigned long ulDpm_win1_map;
	 unsigned long ulDpm_win2_end;
	 unsigned long ulDpm_win2_map;
	 unsigned long ulDpm_win3_end;
	 unsigned long ulDpm_win3_map;
	 unsigned long ulDpm_win4_end;
	 unsigned long ulDpm_win4_map;


} DPM_CONFIGURATION_T;

typedef struct
{
	unsigned long ulIdpmCfg0x0;
	unsigned long ulIdpmAddrCfg;

/*
	 //in case if needed
	 unsigned long ulIdpm_win1_end;
	 unsigned long ulIdpm_win1_map;
	 unsigned long ulIdpm_win2_end;
	 unsigned long ulIdpm_win2_map;
	 unsigned long ulIdpm_win3_end;
	 unsigned long ulIdpm_win3_map;
	 unsigned long ulIdpm_win4_end;
	 unsigned long ulIdpm_win4_map;
*/

} IDPM_CONFIGURATION_T;


typedef struct
{
	unsigned long ulHifIoCfg;
	unsigned long ulDPMEnable;
	DPM_CONFIGURATION_T tDpmConfig;
	unsigned long ulIDPM0Enable;
	IDPM_CONFIGURATION_T tIdpm0Config;
	unsigned long ulIDPM1Enable;
	IDPM_CONFIGURATION_T tIdpm1Config;
} HIF_CONFIG_T;

typedef struct
{
	unsigned long ulHifIoCfg;
	DPM_CONFIGURATION_T tDpmConfig;
} DEFAULT_HIF_CONFIG_T;

void set_dpm_message(char* pcDest, const char* pcDpmType);
#endif
