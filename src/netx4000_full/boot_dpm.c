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

#include "bootblock_oldstyle.h"
#include "netx_lock.h"
#include "options.h"
#include "rdy_run.h"
#include "romvector.h"
#include "sha384.h"
#include "trace.h"
#include "tools.h"

/* Do not use the real OTP fuses on the SCIT board. They can not be defined
 * completely and will trigger a security error in all cases.
 */
#if CFG_ARTIFICIAL_OTP_FUSES!=0
#       include "otp_fuses_scit_board.h"
#endif


/*-----------------------------------*/


#define DPM_BOOT_NETX_RECEIVED_CMD      0x01
#define DPM_BOOT_NETX_SEND_CMD          0x02

#define DPM_BOOT_HOST_SEND_CMD          0x01
#define DPM_BOOT_HOST_RECEIVED_CMD      0x02


#define SRT_HANDSHAKE_REG_ARM_DATA      16
#define SRT_HANDSHAKE_REG_PC_DATA       24



typedef struct
{
	volatile unsigned long aulHandshakeReg[16];
} HANDSHAKE_BLOCK_AREA_T;

#define NX4000_DEF_ptHandshakeArea HANDSHAKE_BLOCK_AREA_T * const ptHandshakeArea = (HANDSHAKE_BLOCK_AREA_T * const)Addr_NX4000_intramhs0;
#define NX4000_DEF_ptHandshakeDtcmArmMirrorArea HANDSHAKE_BLOCK_AREA_T * const ptHandshakeDtcmArmMirrorArea = (HANDSHAKE_BLOCK_AREA_T * const)Addr_NX4000_intramhs0_arm_mirror;


/*-------------------------------------------------------------------------*/


BOOT_DPM_AREA_T  tBootDpmArea  __attribute__ ((section (".boot_dpm_area")));
BOOT_PCIE_AREA_T tBootPcieArea __attribute__ ((section (".boot_pcie_area")));


/*-------------------------------------------------------------------------*/

static const unsigned long aul_dpm_size[16] =
{
	0x00000000,  /* 0000    reserved */
	0x00000000,  /* 0001    reserved */
	0x00000800,  /* 0010    2KB address range    DPM_A[10:0]    +9 PIOs: DPM_A[19:11]  */
	0x00001000,  /* 0011    4KB address range    DPM_A[11:0]    +8 PIOs: DPM_A[19:12]  */
	0x00002000,  /* 0100    8KB address range    DPM_A[12:0]    +7 PIOs: DPM_A[19:13]  */
	0x00004000,  /* 0101    16KB address range   DPM_A[13:0]    +6 PIOs: DPM_A[19:14]  */
	0x00008000,  /* 0110    32KB address range   DPM_A[14:0]    +5 PIOs: DPM_A[19:15]  */
	0x00010000,  /* 0111    64KB address range   DPM_A[15:0]    +4 PIOs: DPM_A[19:16]  */
	0x00020000,  /* 1000    128KB address range  DPM_A[16:0]    +3 PIOs: DPM_A[19:17]  */
	0x00040000,  /* 1001    256KB address range  DPM_A[17:0]    +2 PIOs: DPM_A[19:18]  */
	0x00080000,  /* 1010    512KB address range  DPM_A[18:0]    +1 PIOs: DPM_A[19]     */
	0x00100000,  /* 1011    1024KB address range DPM_A[19:0]    all address lines used */
	0x00000000,  /* 1100    reserved */
	0x00000000,  /* 1101    reserved */
	0x00000000,  /* 1110    reserved */
	0x00000000   /* 1111    reserved */
};



/*                                                 0        1         2         3         4         5    5
 *                                                 1234567890123456789012345678901234567890123456789012345 */
static const char acId_RomCodeDpmListening[56]  = "netX4000 ROM code DPM listening                        ";
static const char acId_RomCodeDpmStopped[56]    = "netX4000 ROM code DPM stopped                          ";



static void init_dpm_memory(DPM_HANDLE_T *ptHandle, unsigned long ulDpmByteSize)
{
	HOSTDEF(ptAsicCtrlArea);
#if CFG_ARTIFICIAL_OTP_FUSES==0
	HOSTDEF(ptRAPSysctrlArea);
#endif
	DPM_BLOCKS_T *ptDpmBlocks;


	/* Get the pointer to the DPM memory. */
	ptDpmBlocks = ptHandle->ptDpmBlocks;

	/* Boot identifier */
	ptDpmBlocks->tInformationBlock.ulDpmBootId = HBOOT_DPM_ID_LISTENING;
	ptDpmBlocks->tInformationBlock.ulDpmByteSize = ulDpmByteSize;

	/* Set the chip ID. */
#if CFG_ARTIFICIAL_OTP_FUSES!=0
	ptDpmBlocks->tInformationBlock.aulChipId[0] = aulScitBoardChipId[0];
	ptDpmBlocks->tInformationBlock.aulChipId[1] = aulScitBoardChipId[1];
	ptDpmBlocks->tInformationBlock.aulChipId[2] = aulScitBoardChipId[2];
	ptDpmBlocks->tInformationBlock.aulChipId[3] = aulScitBoardChipId[3];
#else
	ptDpmBlocks->tInformationBlock.aulChipId[0] = ptRAPSysctrlArea->aulRAP_SYSCTRL_CHIP_ID_[0];
	ptDpmBlocks->tInformationBlock.aulChipId[1] = ptRAPSysctrlArea->aulRAP_SYSCTRL_CHIP_ID_[1];
	ptDpmBlocks->tInformationBlock.aulChipId[2] = ptRAPSysctrlArea->aulRAP_SYSCTRL_CHIP_ID_[2];
	ptDpmBlocks->tInformationBlock.aulChipId[3] = ptRAPSysctrlArea->aulRAP_SYSCTRL_CHIP_ID_[3];
#endif

	/* Set the ASR ID. */
	ptDpmBlocks->tInformationBlock.aulAsrId[0] = ptAsicCtrlArea->aulAsr_id[0];
	ptDpmBlocks->tInformationBlock.aulAsrId[1] = ptAsicCtrlArea->aulAsr_id[1];
	ptDpmBlocks->tInformationBlock.aulAsrId[2] = ptAsicCtrlArea->aulAsr_id[2];
	ptDpmBlocks->tInformationBlock.aulAsrId[3] = ptAsicCtrlArea->aulAsr_id[3];
	ptDpmBlocks->tInformationBlock.aulAsrId[4] = ptAsicCtrlArea->aulAsr_id[4];
	ptDpmBlocks->tInformationBlock.aulAsrId[5] = ptAsicCtrlArea->aulAsr_id[5];
	ptDpmBlocks->tInformationBlock.aulAsrId[6] = ptAsicCtrlArea->aulAsr_id[6];
	ptDpmBlocks->tInformationBlock.aulAsrId[7] = ptAsicCtrlArea->aulAsr_id[7];
	ptDpmBlocks->tInformationBlock.aulAsrId[8] = ptAsicCtrlArea->aulAsr_id[8];
	ptDpmBlocks->tInformationBlock.aulAsrId[9] = ptAsicCtrlArea->aulAsr_id[9];

	/* The DPM is listening now. */
	memcpy(ptDpmBlocks->tInformationBlock.acId, acId_RomCodeDpmListening, sizeof(acId_RomCodeDpmListening));

	/* Initialize the buffer structures */
	ptDpmBlocks->tInformationBlock.ulHostToNetxDataSize = 0;
	ptDpmBlocks->tInformationBlock.ulNetxToHostDataSize = 0;
}



/*-------------------------------------------------------------------------*/



static void dpm_set_state(DPM_HANDLE_T *ptHandle)
{
	HOSTDEF(ptAsicCtrlArea);
	unsigned long ulValue;


	/* Get the old register value. */
	ulValue  = ptAsicCtrlArea->ulNetx_status;
	/* Clear the NETX_STA_CODE and NETX_STATE fields. */
	ulValue &= ~(HOSTMSK(netx_status_NETX_STA_CODE) | HOSTMSK(netx_status_NETX_STATE));
	/* Add the status code. */
	ulValue |= ptHandle->ulNetxStatusCode << HOSTSRT(netx_status_NETX_STA_CODE);
	/* Set the state. */
	ulValue |= ptHandle->ulNetxState << HOSTSRT(netx_status_NETX_STATE);
	ptAsicCtrlArea->ulNetx_status = ulValue;
}



static void pcie_set_state(DPM_HANDLE_T *ptHandle)
{
	unsigned long ulValue;


	/* Get the old register value. */
	ulValue  = tBootPcieArea.tPseudoConfigurationRegisters.ulIdpm_sys_sta;
	/* Clear the NETX_STA_CODE and NETX_STATE fields. */
	ulValue &= ~(HOSTMSK(netx_status_NETX_STA_CODE) | HOSTMSK(netx_status_NETX_STATE));
	/* Add the status code. */
	ulValue |= ptHandle->ulNetxStatusCode << HOSTSRT(netx_status_NETX_STA_CODE);
	/* Set the state. */
	ulValue |= ptHandle->ulNetxState << HOSTSRT(netx_status_NETX_STATE);
	tBootPcieArea.tPseudoConfigurationRegisters.ulIdpm_sys_sta = ulValue;
}



static unsigned long dpm_get_state_differences(void)
{
	HOSTDEF(ptAsicCtrlArea);
	unsigned long ulValue;
	unsigned long ulNetxState;
	unsigned long ulHostState;
	unsigned long ulBitflip;


	/* The netX status register has 4 host bits and 2 netX bits.
	 * BITFLIP boot is executed if host bit #3 and netX bit #1 are equal.
	 */
	ulValue = ptAsicCtrlArea->ulNetx_status;
	/* The netX state has 2 bits. Use bit 1 from this field. */
	ulNetxState = (ulValue & HOSTMSK(netx_status_NETX_STATE))    >> (HOSTSRT(netx_status_NETX_STATE) + 1);
	/* The host state has 4 bits. Use bit 3 from this field. */
	ulHostState = (ulValue & HOSTMSK(netx_status_HOST_STATE_ro)) >> (HOSTSRT(netx_status_HOST_STATE_ro) + 3);
	/* XOR both states. */
	ulBitflip  = ulNetxState ^ ulHostState;
	/* Mask out all but the result bit. */
	ulBitflip &= 1;
	/* Result is 0 if the bits in the host and netX state are the same. */
	return ulBitflip;
}



static unsigned long idpm0_get_state_differences(void)
{
	HOSTDEF(ptIdpm0Area);
	unsigned long ulValue;
	unsigned long ulNetxState;
	unsigned long ulHostState;
	unsigned long ulBitflip;


	/* The netX status register has 4 host bits and 2 netX bits.
	 * BITFLIP boot is executed if host bit #3 and netX bit #1 are equal.
	 */
	ulValue = ptIdpm0Area->ulIdpm_sys_sta;
	/* The netX state has 2 bits. Use bit 1 from this field. */
	ulNetxState = (ulValue & HOSTMSK(idpm_sys_sta_NETX_STATE_ro)) >> (HOSTSRT(idpm_sys_sta_NETX_STATE_ro) + 1);
	/* The host state has 4 bits. Use bit 3 from this field. */
	ulHostState = (ulValue & HOSTMSK(idpm_sys_sta_HOST_STATE))    >> (HOSTSRT(idpm_sys_sta_HOST_STATE) + 3);
	/* XOR both states. */
	ulBitflip  = ulNetxState ^ ulHostState;
	/* Mask out all but the result bit. */
	ulBitflip &= 1;
	/* Result is 0 if the bits in the host and netX state are the same. */
	return ulBitflip;
}



static unsigned long pcie_get_state_differences(void)
{
	unsigned long ulValue;
	unsigned long ulNetxState;
	unsigned long ulHostState;
	unsigned long ulBitflip;


	/* The netX status register has 4 host bits and 2 netX bits.
	 * BITFLIP boot is executed if host bit #3 and netX bit #1 are equal.
	 */
	ulValue = tBootPcieArea.tPseudoConfigurationRegisters.ulIdpm_sys_sta;
	/* The netX state has 2 bits. Use bit 1 from this field. */
	ulNetxState = (ulValue & HOSTMSK(idpm_sys_sta_NETX_STATE_ro)) >> (HOSTSRT(idpm_sys_sta_NETX_STATE_ro) + 1);
	/* The host state has 4 bits. Use bit 3 from this field. */
	ulHostState = (ulValue & HOSTMSK(idpm_sys_sta_HOST_STATE))    >> (HOSTSRT(idpm_sys_sta_HOST_STATE) + 3);
	/* XOR both states. */
	ulBitflip  = ulNetxState ^ ulHostState;
	/* Mask out all but the result bit. */
	ulBitflip &= 1;
	/* Result is 0 if the bits in the host and netX state are the same. */
	return ulBitflip;
}



/*---------------------------------------------------------------------------*/


static void dpm_deinit_registers(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHifIoCtrlArea);
	HOSTDEF(ptDpmArea);
	HOSTDEF(ptHandshakeCtrl0Area);
	size_t sizCnt;


	/*
	 * disable the interface
	 */

	/* Configure the HIF pins. */
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
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



static void idpm0_deinit_registers(void)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptIdpm0Area);
	HOSTDEF(ptHandshakeCtrl0Area);
	size_t sizCnt;
	unsigned long ulValue;


	/*
	 * disable the interface
	 */

	/* Allow R/W accesses only from the R7 to the handshake area 0. */
	ulValue  = HOSTMSK(firewall_cfg_netx_ramhs0_wp_cr7);
	ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_rp_cr7);
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptAsicCtrlArea->aulFirewall_cfg_netx_ramhs[0] = ulValue;
	__IRQ_UNLOCK__;

	/* Disable all windows and write protect them. */
	ptIdpm0Area->ulIdpm_win1_end = 0U;
	ptIdpm0Area->ulIdpm_win1_map = HOSTMSK(dpm_win1_map_wp_cfg_win);
	ptIdpm0Area->ulIdpm_win2_end = 0U;
	ptIdpm0Area->ulIdpm_win2_map = 0U;
	ptIdpm0Area->ulIdpm_win3_end = 0U;
	ptIdpm0Area->ulIdpm_win3_map = 0U;
	ptIdpm0Area->ulIdpm_win4_end = 0U;
	ptIdpm0Area->ulIdpm_win4_map = 0U;

	/* Disable the tunnel and write protect it. */
	ptIdpm0Area->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_cfg_win);

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
	ptIdpm0Area->ulIdpm_irq_arm_mask_reset = 0xffffffff;
	ptIdpm0Area->ulIdpm_irq_pci_inta_mask_reset = 0xffffffff;
	ptIdpm0Area->ulIdpm_irq_pci_intb_mask_reset = 0xffffffff;
	ptIdpm0Area->ulIdpm_irq_pci_intc_mask_reset = 0xffffffff;
	ptIdpm0Area->ulIdpm_irq_pci_intd_mask_reset = 0xffffffff;
	ptIdpm0Area->ulIdpm_firmware_irq_mask = 0;
}



/*---------------------------------------------------------------------------*/


static void dpm_init(DPM_HANDLE_T *ptHandle, DPM_TRANSPORT_TYPE_T tDpmTransportType)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptDpmArea);
	HOSTDEF(ptHifIoCtrlArea);
	HOSTDEF(ptHandshakeCtrl0Area);
	HOSTDEF(ptHandshakeArea);
	HOSTDEF(ptHandshakeDtcmArmMirrorArea);
	unsigned long ulValue;
	unsigned long ulNetxAdr;


	ptHandle->ptDpmBlocks = &(tBootDpmArea.tDpmBlocks);
	ptHandle->pulBitflipArea = tBootDpmArea.aulBitflipArea;
	ptHandle->pfnSetState = dpm_set_state;
	ptHandle->pfnGetStateDifferences = dpm_get_state_differences;
	ptHandle->pulHandshakeRegister = &(ptHandshakeDtcmArmMirrorArea->aulHandshakeReg[0]);
	ptHandle->uiNetxToHostBufferIdx = 0;
	ptHandle->uiHostToNetxBufferIdx = 0;
	ptHandle->ulNetxState = 0;
	ptHandle->ulNetxStatusCode = HBOOT_DPM_BOOT_VERSION;
	ptHandle->tConnection = DPM_CONNECTION_DPM;

	/* Enable the DPM clock. */
	ulValue  = ptAsicCtrlArea->ulClock_enable;
	ulValue |= HOSTMSK(clock_enable_dpm);
	__IRQ_LOCK__;
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptAsicCtrlArea->ulClock_enable = ulValue;
	__IRQ_UNLOCK__;

	/* Disable the DPM for new configuration. */
	dpm_deinit_registers();

	/* DPM mapping:
	 * 0x0000 - 0x00ff : DPM configuration area
	 * 0x0100 - 0x017f : dpm_mb_start
	 * 0x0180 - 0x01ff : intramhs_dpm_mirror
	 * 0x0200 - 0xffff : dpm_mb_start+0x0100
	 */
	ptDpmArea->ulDpm_win1_end = 0x017fU + 1;
	ulNetxAdr = (unsigned long)(&(tBootDpmArea.tDpmBlocks));
	ulValue  = (ulNetxAdr-0x0100U) & HOSTMSK(dpm_win1_map_win_map);
	ulValue |= ulNetxAdr & HOSTMSK(dpm_win1_map_win_page);
	ulValue |= HOSTMSK(dpm_win1_map_wp_cfg_win);
	ulValue |= HOSTMSK(dpm_win1_map_read_ahead);
	ulValue |= HOSTMSK(dpm_win1_map_byte_area);
	ptDpmArea->ulDpm_win1_map = ulValue;

	ptDpmArea->ulDpm_win2_end = 0x01ffU + 1;
	ulNetxAdr = HOSTADDR(intramhs_dpm_mirror);
	ulValue  = (ulNetxAdr-0x0180U) & HOSTMSK(dpm_win1_map_win_map);
	ulValue |= ulNetxAdr & HOSTMSK(dpm_win1_map_win_page);
	ulValue |= HOSTMSK(dpm_win1_map_byte_area);
	ptDpmArea->ulDpm_win2_map = ulValue;

	ptDpmArea->ulDpm_win3_end = 0xffffU + 1;
	ulNetxAdr = ((unsigned long)(&(tBootDpmArea.tDpmBlocks)))+0x100;
	ulValue  = (ulNetxAdr-0x0200U) & HOSTMSK(dpm_win1_map_win_map);
	ulValue |= ulNetxAdr & HOSTMSK(dpm_win1_map_win_page);
	ulValue |= HOSTMSK(dpm_win1_map_read_ahead);
	ulValue |= HOSTMSK(dpm_win1_map_byte_area);
	ptDpmArea->ulDpm_win3_map = ulValue;

	ptDpmArea->ulDpm_win4_end = 0U;
	ptDpmArea->ulDpm_win4_map = 0U;


	/* Disable the tunnel and write protect it. */
	ptDpmArea->ulDpm_tunnel_cfg = HOSTMSK(dpm_tunnel_cfg_wp_cfg_win);


	/*
	 * setup the netX parameter area
	 */
	/* DPM size in bytes. */
	ulValue  = g_t_romloader_options.t_hif_options.ucDpmAddrCfg;
	ulValue &= HOSTMSK(dpm_addr_cfg_addr_range);
	init_dpm_memory(ptHandle, aul_dpm_size[ulValue]);


	/* Setup the handshake cell
	 *   DPM write data in data bits 31..24, bits 23..16 are read-only.
	 *   ARM write data in data bits 23..16, bits 31..24 are read-only.
	 */
	/* locate the handshake cells to offset 0 in INTRAM5 */
	ptHandshakeCtrl0Area->ulHandshake_base_addr = HOSTMSK(handshake_base_addr_enable) | (0<<HOSTSRT(handshake_base_addr_base256));
	/* set handshake cell to 8 bit, DPM<->ARM */
	ulValue  = 0<<HOSTSRT(handshake_hsc0_ctrl_width);
	ulValue |= 1<<HOSTSRT(handshake_hsc0_ctrl_mode);
	ptHandshakeCtrl0Area->aulHandshake_hsc_ctrl[0]= ulValue;
	/* clear the handshake cell */
	ptHandshakeArea->aulHandshakeReg[0] = 0;


	/* configure DPM */
	ptDpmArea->ulDpm_cfg0x0       = g_t_romloader_options.t_hif_options.ucDpmCfg0x0;
	ptDpmArea->ulDpm_addr_cfg     = g_t_romloader_options.t_hif_options.ucDpmAddrCfg;
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
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptHifIoCtrlArea->ulHif_io_cfg = ulValue;
	__IRQ_UNLOCK__;

	/* Set the port control configuration for all HIF and MEM pins. */
	setup_hif_and_mem_portctrl();
}



static void idpm0_init(DPM_HANDLE_T *ptHandle)
{
	HOSTDEF(ptIdpm0Area);
	HOSTDEF(ptHandshakeCtrl0Area);
	HOSTDEF(ptHandshakeArea);
	HOSTDEF(ptHandshakeDtcmArmMirrorArea);
	unsigned long ulValue;
	unsigned long ulNetxAdr;

	ptHandle->ptDpmBlocks = &(tBootDpmArea.tDpmBlocks);
	ptHandle->pulBitflipArea = tBootDpmArea.aulBitflipArea;
	ptHandle->pfnSetState = dpm_set_state;
	ptHandle->pfnGetStateDifferences = idpm0_get_state_differences;
	ptHandle->pulHandshakeRegister = &(ptHandshakeDtcmArmMirrorArea->aulHandshakeReg[0]);
	ptHandle->uiNetxToHostBufferIdx = 0;
	ptHandle->uiHostToNetxBufferIdx = 0;
	ptHandle->ulNetxState = 0;
	ptHandle->ulNetxStatusCode = HBOOT_DPM_BOOT_VERSION;
	ptHandle->tConnection = DPM_CONNECTION_IDPM;

	/* Disable the DPM for new configuration. */
	idpm0_deinit_registers();

	/* DPM mapping:
	 * 0x0000 - 0x00ff : DPM configuration area
	 * 0x0100 - 0x017f : dpm_mb_start
	 * 0x0180 - 0x01ff : intramhs_dpm_mirror
	 * 0x0200 - 0xffff : dpm_mb_start+0x0100
	 */
	ptIdpm0Area->ulIdpm_win1_end = 0x017fU + 1;
	ulNetxAdr = (unsigned long)(&(tBootDpmArea.tDpmBlocks));
	ulValue  = (ulNetxAdr-0x0100U) & HOSTMSK(idpm_win1_map_win_map);
	ulValue |= HOSTMSK(idpm_win1_map_wp_cfg_win);
	ptIdpm0Area->ulIdpm_win1_map = ulValue;

	ptIdpm0Area->ulIdpm_win2_end = 0x01ffU + 1;
	ulNetxAdr = HOSTADDR(intramhs_dpm_mirror);
	ulValue  = (ulNetxAdr-0x0180U) & HOSTMSK(idpm_win1_map_win_map);
	ptIdpm0Area->ulIdpm_win2_map = ulValue;

	ptIdpm0Area->ulIdpm_win3_end = 0xffffU + 1;
	ulNetxAdr = ((unsigned long)(&(tBootDpmArea.tDpmBlocks)))+0x100;
	ulValue  = (ulNetxAdr-0x0200U) & HOSTMSK(idpm_win1_map_win_map);
	ptIdpm0Area->ulIdpm_win3_map = ulValue;

	ptIdpm0Area->ulIdpm_win4_end = 0U;
	ptIdpm0Area->ulIdpm_win4_map = 0U;


	/* Disable the tunnel and write protect it. */
	ptIdpm0Area->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_cfg_win);


	/*
	 * setup the netX parameter area
	 */
	init_dpm_memory(ptHandle, 0x00010000);

	/* Setup the handshake cell
	 *   DPM write data in data bits 31..24, bits 23..16 are read-only.
	 *   ARM write data in data bits 23..16, bits 31..24 are read-only.
	 */
	/* locate the handshake cells to offset 0 in INTRAM5 */
	ptHandshakeCtrl0Area->ulHandshake_base_addr = HOSTMSK(handshake_base_addr_enable) | (0<<HOSTSRT(handshake_base_addr_base256));
	/* set handshake cell to 8 bit, DPM<->ARM */
	ulValue  = 0<<HOSTSRT(handshake_hsc0_ctrl_width);
	ulValue |= 1<<HOSTSRT(handshake_hsc0_ctrl_mode);
	ptHandshakeCtrl0Area->aulHandshake_hsc_ctrl[0]= ulValue;
	/* clear the handshake cell */
	ptHandshakeArea->aulHandshakeReg[0] = 0;

	/* configure IRQs */
	ulValue = 0;
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTA_CFG_DPM_Sw)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_inta_mask_set_dpm_sw);
	}
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTA_CFG_Firmware)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_inta_mask_set_firmware);
	}
	ptIdpm0Area->ulIdpm_irq_pci_inta_mask_set = ulValue;

	ulValue = 0;
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTB_CFG_DPM_Sw)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_intb_mask_set_dpm_sw);
	}
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTB_CFG_Firmware)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_intb_mask_set_firmware);
	}
	ptIdpm0Area->ulIdpm_irq_pci_intb_mask_set = ulValue;

	ulValue = 0;
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTC_CFG_DPM_Sw)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_intc_mask_set_dpm_sw);
	}
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTC_CFG_Firmware)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_intc_mask_set_firmware);
	}
	ptIdpm0Area->ulIdpm_irq_pci_intc_mask_set = ulValue;

	ulValue = 0;
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTD_CFG_DPM_Sw)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_intd_mask_set_dpm_sw);
	}
	if( (g_t_romloader_options.t_hif_options.ucPcieIntCfg&PCIE_INTD_CFG_Firmware)!=0 )
	{
		ulValue |= HOSTMSK(idpm_irq_pci_intd_mask_set_firmware);
	}
	ptIdpm0Area->ulIdpm_irq_pci_intd_mask_set = ulValue;


	/* Enable the DPM.
	 * Enable the configuration window at offset 0.
	 */
	ulValue  = HOSTMSK(idpm_cfg0x0_enable);
	ptIdpm0Area->ulIdpm_cfg0x0 = ulValue;
}



static BOOTING_T pcie_init(DPM_HANDLE_T *ptHandle)
{
	HOSTDEF(ptRAPSysctrlArea);
	HOSTDEF(ptPCIEArea);
	HOSTDEF(ptIdpm0Area);
	HOSTDEF(ptAsicCtrlArea);
	BOOTING_T tResult;
	unsigned long ulValue;
	TIMER_HANDLE_T tHandle;
	int iElapsed;


	ptHandle->ptDpmBlocks = &(tBootPcieArea.tDpmBlocks);
	ptHandle->pulBitflipArea = tBootPcieArea.aulBitflipArea;
	ptHandle->pfnSetState = pcie_set_state;
	ptHandle->pfnGetStateDifferences = pcie_get_state_differences;
	ptHandle->pulHandshakeRegister = &(tBootPcieArea.tDpmBlocks.tHandshakeBlock.aulHandshakeReg[0]);
	ptHandle->uiNetxToHostBufferIdx = 0;
	ptHandle->uiHostToNetxBufferIdx = 0;
	ptHandle->ulNetxState = 0;
	ptHandle->ulNetxStatusCode = HBOOT_DPM_BOOT_VERSION;
	ptHandle->tConnection = DPM_CONNECTION_PCIE;

	/* Copy the complete IDPM0 area to the pseudo configuration area. */
	memcpy(&(tBootPcieArea.tPseudoConfigurationRegisters), ptIdpm0Area, sizeof(tBootPcieArea.tPseudoConfigurationRegisters));
	/* Clear the start of the netX -> host area. */
	memset(&(tBootPcieArea.tDpmBlocks.tMailboxBlock), 0, 0x40);

	/* Initialize the netX version pseudo configuration registers. */
	ulValue = ptAsicCtrlArea->ulNetx_version;
	tBootPcieArea.tPseudoConfigurationRegisters.ulIdpm_netx_version = ulValue;

	/* The DPM size is fixed to 64kByte. */
	ulValue = 0x00010000;
	init_dpm_memory(ptHandle, ulValue);

	/* Initialize the handshake cell. */
	tBootPcieArea.tDpmBlocks.tHandshakeBlock.aulHandshakeReg[0] = 0;
	/* Initialize one cell after the handshake register to prevent X in the 64bit accesses. */
	tBootPcieArea.tDpmBlocks.tHandshakeBlock.aulHandshakeReg[1] = 0;

	/* Initialize the netX -> host buffer to prevent X in the 64bit accesses. */
	tBootPcieArea.tDpmBlocks.tMailboxBlock.aucNetxToHostData[0] = 0;
	tBootPcieArea.tDpmBlocks.tMailboxBlock.aucNetxToHostData[1] = 0;

	/* Expect success. */
	tResult = BOOTING_Ok;

	/* Test if the PCIe core is already enabled.
	 * This is the case if the clock configuration is already enabled.
	 */
	ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG;
	ulValue &= HOSTMSK(RAP_SYSCTRL_CLKCFG_PCIe);
	if( ulValue!=0 )
	{
		trace_message(TRACEMSG_DPM_PcieStillActive);

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
				trace_message_ul(TRACEMSG_DPM_PciePowerNotActive, g_t_romloader_options.tPcieOptions.ulPowerUpTimeoutTicks);
				tResult = BOOTING_Setup_Error;
				break;
			}
			else
			{
				ulValue  = ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT;
				ulValue &= HOSTMSK(RAP_SYSCTRL_NOCPWRSTAT_PCIe);
			}
		} while( ulValue==0 );

		/* Set the mapping of the PCIe DPM back to RAP_INTRAM3. */
		ptPCIEArea->ulPCIE_AWMASK0 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[0].ulAxiWindowMask;
		ptPCIEArea->ulPCIE_ADEST0  = g_t_romloader_options.tPcieOptions.atWindowConfiguration[0].ulAxiDestination;
		ptPCIEArea->ulPCIE_AWBASE0 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[0].ulAxiWindowBase;
	}
	else
	{
		/* The PCIe core was not enabled yet. */

		/* RAP_SYSCTRL_PcieSetup(0, true); */
		ulValue  = (unsigned long)(g_t_romloader_options.tPcieOptions.usSysctrlPcieCfg);
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
				trace_message(TRACEMSG_DPM_PciePowerMaskedOut);
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
						trace_message_ul(TRACEMSG_DPM_PciePowerNotActive, g_t_romloader_options.tPcieOptions.ulPowerUpTimeoutTicks);
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

			/* Note: Spec says we have to wait 5 milliseconds between clock enable and reset release! */
			cr7_global_timer_delay_ticks(g_t_romloader_options.tPcieOptions.ulResetDelayTicks);

			/* pcie_test(0, Addr_intram0, Addr_NX2RAP_pci) */
			/* pcie_init_ep(ulRxAdr, ulTxAdr); */

			//sim_message("Performing software reset of PCIE", disp_only, 0);

			/* enable configuration registers only */
			ulValue  = HOSTMSK(PCIE_SW_RST_RST_CFG_B);
			ulValue |= HOSTMSK(PCIE_SW_RST_RST_LOAD_B);
			ptPCIEArea->ulPCIE_SW_RST = ulValue;

			/* Set the device ID. */
			ptPCIEArea->ulPCIE_PCI_CR_ID = g_t_romloader_options.tPcieOptions.ulVidPid;
			ptPCIEArea->ulPCIE_PCI_CR_CLASS_REV = g_t_romloader_options.tPcieOptions.ulRevisionClassCode;
			ptPCIEArea->ulPCIE_PCI_CR_SSID = g_t_romloader_options.tPcieOptions.ulSubVidSubPid;

			/* Set the link capabilities. */
			ulValue  = ptPCIEArea->ulPCIE_PCI_CR_LINK_CAP;
			/* Clear all supported link speeds. */
			ulValue &= ~0x0000000fU;
			/* Set the supported speeds. */
			ulValue |=  ((unsigned long)g_t_romloader_options.tPcieOptions.ucSupportedLinkSpeeds) & 0x0000000fU;
			ptPCIEArea->ulPCIE_PCI_CR_LINK_CAP = ulValue;

			/* The six Base Address Registers have the following functions:
			 * - BAR0 and BAR1 span a 64bit user defined memory space.
			 *   The four AXI (RX) windows are accessible through this memory space.
			 *   Set the required size of the memory space by writing the BAR Mask0 register.
			 * - BAR1 and BAR2 define a 64bit register space for the PCIE registers.
			 *   Registers of the PCI configuration space are not accessible through this window.
			 *   The window size is fixed to 4K. Default BAR Mask1 values should not be changed.
			 * - BAR2 and BAR3 are reserved an can not be used.
			 */
			/* set maximum supported data size per request */
			ptPCIEArea->ulPCIE_PCI_CR_BASE_SIZE00 = g_t_romloader_options.tPcieOptions.ulBaseSize00;

			/* Disable all windows. */
			ptPCIEArea->ulPCIE_AWBASE0 = 0;
			ptPCIEArea->ulPCIE_AWBASE1 = 0;
			ptPCIEArea->ulPCIE_AWBASE2 = 0;
			ptPCIEArea->ulPCIE_AWBASE3 = 0;

			/* Setup and enable all windows. */
			ptPCIEArea->ulPCIE_AWMASK0 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[0].ulAxiWindowMask;
			ptPCIEArea->ulPCIE_ADEST0  = g_t_romloader_options.tPcieOptions.atWindowConfiguration[0].ulAxiDestination;
			ptPCIEArea->ulPCIE_AWBASE0 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[0].ulAxiWindowBase;

			ptPCIEArea->ulPCIE_AWMASK1 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[1].ulAxiWindowMask;
			ptPCIEArea->ulPCIE_ADEST1  = g_t_romloader_options.tPcieOptions.atWindowConfiguration[1].ulAxiDestination;
			ptPCIEArea->ulPCIE_AWBASE1 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[1].ulAxiWindowBase;

			ptPCIEArea->ulPCIE_AWMASK2 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[2].ulAxiWindowMask;
			ptPCIEArea->ulPCIE_ADEST2  = g_t_romloader_options.tPcieOptions.atWindowConfiguration[2].ulAxiDestination;
			ptPCIEArea->ulPCIE_AWBASE2 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[2].ulAxiWindowBase;

			ptPCIEArea->ulPCIE_AWMASK3 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[3].ulAxiWindowMask;
			ptPCIEArea->ulPCIE_ADEST3  = g_t_romloader_options.tPcieOptions.atWindowConfiguration[3].ulAxiDestination;
			ptPCIEArea->ulPCIE_AWBASE3 = g_t_romloader_options.tPcieOptions.atWindowConfiguration[3].ulAxiWindowBase;

			/* Set the size of the bar. This should be enough for all windows. */
			ptPCIEArea->asPCIE_PCI_CR_BAR_MSK0[0].ulL = g_t_romloader_options.tPcieOptions.ulBaseAddressRegisterMask00Lower;
			ptPCIEArea->asPCIE_PCI_CR_BAR_MSK0[0].ulU = g_t_romloader_options.tPcieOptions.ulBaseAddressRegisterMask00Upper;


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

			trace_message(TRACEMSG_DPM_PcieWaitingForLinkUp);
			do
			{
				ulValue  = ptPCIEArea->ulPCIE_PCI_CORE_STATUS_1;
				ulValue &= HOSTMSK(PCIE_PCI_CORE_STATUS_1_DL_DOWN_STATUS);
			} while( ulValue!=0 );
			trace_message(TRACEMSG_DPM_PcieLinkIsUp);
		}
	}

	if( tResult==BOOTING_Ok )
	{
		/* Set the "unlocked" bit. */
		ulValue  = HOSTMSK(idpm_status_unlocked);
		tBootPcieArea.tPseudoConfigurationRegisters.ulIdpm_status = ulValue;
	}

	return tResult;
}



/*-----------------------------------*/


/* wait for harmony ^_^ (purr...) */
static unsigned long dpm_is_harmony(DPM_HANDLE_T *ptHandle, unsigned long ulMask)
{
	unsigned long ulHsVal;
	unsigned long ulValue;


	/* Read the handshake register only once. */
	ulHsVal  = *(ptHandle->pulHandshakeRegister);
	/* Get the netX part. */
	ulValue  = ulHsVal >> SRT_HANDSHAKE_REG_ARM_DATA;
	/* Invert it with the host part. */
	ulValue ^= ulHsVal >> SRT_HANDSHAKE_REG_PC_DATA;
	/* Only check the masked part. */
	ulValue &= ulMask;

	return ulValue;
}



static DPM_ConnectionState_t dpm_pollConnection(DPM_HANDLE_T *ptHandle)
{
	unsigned long ulValue;


	ulValue = dpm_is_harmony(ptHandle, DPM_BOOT_NETX_RECEIVED_CMD);
	return (ulValue==0) ? DPM_ConnectionState_Idle : DPM_ConnectionState_Connected;

}



static unsigned int dpm_peek(DPM_HANDLE_T *ptHandle)
{
	/* Return the number of chars still waiting in the receive buffer. */
	return ptHandle->ptDpmBlocks->tInformationBlock.ulHostToNetxDataSize - ptHandle->uiHostToNetxBufferIdx;
}



static void dpm_flush(DPM_HANDLE_T *ptHandle)
{
	unsigned long ulValue;


	/* at least one char waiting? */
	if( ptHandle->uiNetxToHostBufferIdx!=0 )
	{
		/* yes -> send the buffer */

		/* set the packet size */
		ptHandle->ptDpmBlocks->tInformationBlock.ulNetxToHostDataSize = ptHandle->uiNetxToHostBufferIdx;

		/* Toggle the 'packet send' flag. */
		*(ptHandle->pulHandshakeRegister) ^= DPM_BOOT_NETX_SEND_CMD<<SRT_HANDSHAKE_REG_ARM_DATA;

		/* Wait for the host acknowledge. */
		do
		{
			ulValue = dpm_is_harmony(ptHandle, DPM_BOOT_NETX_SEND_CMD);
		} while( ulValue!=0 );

		/* Reset the buffer index. */
		ptHandle->uiNetxToHostBufferIdx = 0;
	}
}



static void dpm_send(DPM_HANDLE_T *ptHandle, unsigned char ucData)
{
	/* Write the char to the buffer. */
	ptHandle->ptDpmBlocks->tMailboxBlock.aucNetxToHostData[ptHandle->uiNetxToHostBufferIdx] = ucData;

	/* Increase the buffer index. */
	++(ptHandle->uiNetxToHostBufferIdx);

	/* buffer full? */
	if( ptHandle->uiNetxToHostBufferIdx>=DPM_NETX_TO_HOST_BUFFERSIZE )
	{
		dpm_flush(ptHandle);
	}
}



static unsigned char dpm_get(DPM_HANDLE_T *ptHandle)
{
	DPM_BLOCKS_T *ptDpmBlocks;
	unsigned long ulValue;
	unsigned char ucData;


	ptDpmBlocks = ptHandle->ptDpmBlocks;

	/* Are still chars in the buffer? */
	while( ptHandle->uiHostToNetxBufferIdx>=ptDpmBlocks->tInformationBlock.ulHostToNetxDataSize )
	{
		/* Reset the buffer index. */
		ptHandle->uiHostToNetxBufferIdx = 0;

		/* Wait for a new packet. */
		do
		{
			ulValue = dpm_is_harmony(ptHandle, DPM_BOOT_NETX_RECEIVED_CMD);
		} while( ulValue==0 );
	}

	/* A new packet with at least one char arrived. */
	ucData = ptDpmBlocks->tMailboxBlock.aucHostToNetxData[ptHandle->uiHostToNetxBufferIdx];

	/* Increment buffer index. */
	++(ptHandle->uiHostToNetxBufferIdx);

	/* Reached the buffer limit? */
	if( ptHandle->uiHostToNetxBufferIdx>=ptDpmBlocks->tInformationBlock.ulHostToNetxDataSize )
	{
		/* Flip the 'packet received bit'. */
		*(ptHandle->pulHandshakeRegister) ^= DPM_BOOT_NETX_RECEIVED_CMD<<SRT_HANDSHAKE_REG_ARM_DATA;
	}

	/* OK */
	return ucData;
}



static void dpm_dump(DPM_HANDLE_T *ptHandle)
{
	/* dump all bytes in the receive buffer */
	while( dpm_peek(ptHandle)!=0 )
	{
		dpm_get(ptHandle);
	}
}


/*-------------------------------------------------------------------------*/


static int transport_interface_dpm_get_dword(unsigned long *pulData, void *pvUser)
{
	HOSTDEF(ptCryptArea);
	DPM_HANDLE_T *ptHandle;
	size_t sizCnt;
	unsigned long ulValue;
	unsigned char ucData;


	ptHandle = (DPM_HANDLE_T*)pvUser;

	sizCnt = 4;
	ulValue = 0;
	do
	{
		ucData = dpm_get(ptHandle);

		ulValue >>= 8U;
		ulValue |= ((unsigned long)ucData) << 24U;
	} while( --sizCnt!=0 );

	sha384_update_ul(ulValue);
	*pulData = ulValue;

	return 0;
}



static int transport_interface_dpm_get_area(unsigned long *pulData, size_t sizDwords, void *pvUser)
{
	unsigned long *pulDataEnd;


	pulDataEnd = pulData + sizDwords;
	while( pulData<pulDataEnd )
	{
		transport_interface_dpm_get_dword(pulData, pvUser);
		++pulData;
	}

	return 0;
}



static int transport_interface_dpm_stop(void *pvUser)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptHandshakeDtcmArmMirrorArea);
	HOSTDEF(ptIdpm0Area);
	HOSTDEF(ptPCIEArea);
	DPM_HANDLE_T *ptHandle;
	int iNetxIsLocked;
	unsigned long ulValue;
	unsigned long ulNetxState;
	unsigned long ulNetxStaCode;
	unsigned long ulHostState;


	/* Cast the user data to the DPM handle. */
	ptHandle = (DPM_HANDLE_T*)pvUser;

	/* Switch to the INTRAMHS0 for a PCIe connection.
	 * NOTE: The firewall was already opened in the idpm0_init routine.
	 */
	if( ptHandle->tConnection==DPM_CONNECTION_PCIE )
	{
		iNetxIsLocked = is_netx_locked();
		if( iNetxIsLocked!=0 )
		{
			/* Get the current status values from the PCIe RAP RAM. */
			ulValue = tBootPcieArea.tPseudoConfigurationRegisters.ulIdpm_sys_sta;
			ulNetxState   = (ulValue & HOSTMSK(idpm_sys_sta_NETX_STATE_ro)) >> HOSTSRT(idpm_sys_sta_NETX_STATE_ro);
			ulNetxStaCode = (ulValue & HOSTMSK(idpm_sys_sta_NETX_STA_CODE_ro)) >> HOSTSRT(idpm_sys_sta_NETX_STA_CODE_ro);
			ulHostState   = (ulValue & HOSTMSK(idpm_sys_sta_HOST_STATE)) >> HOSTSRT(idpm_sys_sta_HOST_STATE);

			/* Apply the NETX_STATE and NETX_STA_CODE to the IDPM area. */
			ulValue  = ptAsicCtrlArea->ulNetx_status;
			ulValue &= ~(HOSTMSK(netx_status_NETX_STA_CODE)|HOSTMSK(netx_status_NETX_STATE));
			ulValue |= ulNetxState << HOSTSRT(netx_status_NETX_STATE);
			ulValue |= ulNetxStaCode << HOSTSRT(netx_status_NETX_STA_CODE);
			ptAsicCtrlArea->ulNetx_status = ulValue;

			/* Apply the HOST_STATE to the IDPM area. */
			ulValue  = ptIdpm0Area->ulIdpm_sys_sta;
			ulValue &= ~HOSTMSK(idpm_sys_sta_HOST_STATE);
			ulValue |= ulHostState << HOSTSRT(idpm_sys_sta_HOST_STATE);
			ptIdpm0Area->ulIdpm_sys_sta = ulValue;


			/* Copy the mailbox handshake register. */
			ptHandshakeDtcmArmMirrorArea->aulHandshakeReg[0] = tBootPcieArea.tDpmBlocks.tHandshakeBlock.aulHandshakeReg[0];


			/* Switch the memory mapping of the PCIe core to the IDPM0. */
			ptPCIEArea->ulPCIE_ADEST0  = HOSTADDR(intramhs0_dpm_mirror);
		}
		else
		{
			trace_message(TRACEMSG_DPM_StoppingLockedPcie);
		}
	}

	/* The DPM is stopped now.
	 * NOTE: This modifies the settings in the RAP RAM. This should be not visible anymore.
	 *       The settings in the IDPM0 are already switched to "stopped". */
	ptHandle->ptDpmBlocks->tInformationBlock.ulDpmBootId = HBOOT_DPM_ID_STOPPED;
	memcpy(ptHandle->ptDpmBlocks->tInformationBlock.acId, acId_RomCodeDpmStopped, sizeof(acId_RomCodeDpmStopped));

	return 0;
}



static int transport_interface_dpm_restart(void *pvUser)
{
	HOSTDEF(ptPCIEArea);
	DPM_HANDLE_T *ptHandle;
	int iNetxIsLocked;


	/* Cast the user data to the DPM handle. */
	ptHandle = (DPM_HANDLE_T*)pvUser;

	/* Reconfigure the IO pins for a DPM connection. */
	if( ptHandle->tConnection==DPM_CONNECTION_DPM )
	{
		/* Set the port control configuration for all HIF and MEM pins. */
		setup_hif_and_mem_portctrl();
	}
	/* Switch back to the simulated DPM in the RAP RAM for a PCIe connection. */
	else if( ptHandle->tConnection==DPM_CONNECTION_PCIE )
	{
		iNetxIsLocked = is_netx_locked();
		if( iNetxIsLocked==0 )
		{
			ptPCIEArea->ulPCIE_ADEST0  = (unsigned long)(&(tBootPcieArea.tDpmBlocks));
		}
	}

	/* The DPM is listening now. */
	ptHandle->ptDpmBlocks->tInformationBlock.ulDpmBootId = HBOOT_DPM_ID_LISTENING;
	memcpy(ptHandle->ptDpmBlocks->tInformationBlock.acId, acId_RomCodeDpmListening, sizeof(acId_RomCodeDpmListening));

	return 0;
}



static const HBOOT_TRANSPORT_INTERFACE_T tTransportInterface_dpm =
{
	transport_interface_dpm_get_dword,
	transport_interface_dpm_get_area,
	transport_interface_dpm_stop,
	transport_interface_dpm_restart,
	NULL,
	NULL
};


/*-------------------------------------------------------------------------*/


static BOOTING_T boot_dpm_bitflip(DPM_HANDLE_T *ptHandle)
{
	BOOTING_T tResult;


	tResult = boot_probe_mmapped_image(ptHandle->pulBitflipArea, transport_interface_dpm_stop, transport_interface_dpm_restart, ptHandle);

	/* Set the status code. */
	ptHandle->ulNetxStatusCode = (unsigned long)tResult;

	/* End of DPM boot sequence -> flip bit 1. */
	ptHandle->ulNetxState ^= 2;

	/* Show the result. */
	ptHandle->pfnSetState(ptHandle);

	return tResult;
}


/*-------------------------------------------------------------------------*/


static BOOTING_T boot_dpm_mailbox(DPM_HANDLE_T *ptHandle)
{
	BOOTBLOCK_OLDSTYLE_U_T tBootblock;
	int iResult;
	BOOTING_T tResult;
	unsigned long ulValue;
	size_t sizHashDwords;


	/* Receive the boot block and build the checksum. */
	iResult = transport_interface_dpm_get_area(tBootblock.aul, sizeof(BOOTBLOCK_OLDSTYLE_U_T)/sizeof(unsigned long), ptHandle);
	if( iResult!=0 )
	{
		/* Failed to receive the boot block. */
		tResult = BOOTING_Transfer_Error;
	}
	else
	{
		ulValue = bootblock_oldstyle_bootblock_checksum(tBootblock.aul);
		if( ulValue!=0 )
		{
			/* Invalid checksum! */
			trace_message(TRACEMSG_BootBlock_ChecksumError);
			tResult = BOOTING_Header_Checksum_Invalid;
		}
		/* test the magic cookie */
		else if( tBootblock.s.ulMagic!=g_t_romloader_options.t_system_config.ulBootBlockOldstyleMagic )
		{
			/* Invalid magic! */
			trace_message_ul(TRACEMSG_BootBlock_NoMagicCookie, tBootblock.s.ulMagic);
			tResult = BOOTING_Cookie_Invalid;
		}
		/* test the signature */
		else if( tBootblock.s.ulSignature!=BOOTBLOCK_HBOOT_SIGNATURE )
		{
			/* no valid signature found */
			trace_message_ul(TRACEMSG_BootBlock_InvalidSignature, tBootblock.s.ulSignature);
			tResult = BOOTING_Signature_Invalid;
		}
		else
		{
			/* The boot block is OK! */

			/* Send response. */
			dpm_send(ptHandle, BOOTING_Ok);
			dpm_flush(ptHandle);

			/* Get the hash size. */
			ulValue  = tBootblock.aul[7];
			ulValue &= 0x0000000fU;
			sizHashDwords = ((size_t)ulValue) + 1U;

			/* process a streamed HBoot image */
			tResult = boot_process_hboot_image(&tTransportInterface_dpm, ptHandle, sizHashDwords);
		}
	}

	/* send status */
	dpm_send(ptHandle, tResult);
	dpm_flush(ptHandle);

	return tResult;
}



static BOOTING_T boot_dpm_common(DPM_HANDLE_T *ptHandle)
{
	SYST_HANDLE_T tBlinkiHandle;
	unsigned long ulValue;
	BOOTING_T tResult;


	/* Expect success. */
	tResult = BOOTING_Ok;

	/* Show the boot loader version. */
	ptHandle->ulNetxState = 2;
	ptHandle->pfnSetState(ptHandle);

	rdy_run_syst_init(&tBlinkiHandle, SYST_M_DPM_WAIT_FOR_CONNECTION, SYST_S_DPM_WAIT_FOR_CONNECTION);

	/* Wait for BITFLIP or MAILBOX booting. */
	while(1)
	{
		/* Blink with the SYS LED. */
		rdy_run_syst(&tBlinkiHandle);

		ulValue = ptHandle->pfnGetStateDifferences();
		if( ulValue==0 )
		{
			trace_message(TRACEMSG_DPM_BitflipModeActivated);

			/* Switch off the LEDs. */
			rdy_run_setLEDs(RDYRUN_OFF);

			/* Old style bit flip boot. */
			tResult = boot_dpm_bitflip(ptHandle);
		}

		if( dpm_pollConnection(ptHandle)==DPM_ConnectionState_Connected )
		{
			trace_message(TRACEMSG_DPM_MailboxModeActivated);

			/* LEDs off */
			rdy_run_setLEDs(RDYRUN_OFF);

			tResult = boot_dpm_mailbox(ptHandle);

			/* dump all remaining data in the last packet */
			dpm_dump(ptHandle);

			/* send error message */
			dpm_send(ptHandle, tResult);
			dpm_flush(ptHandle);
		}
	}

	return tResult;
}



BOOTING_T boot_dpm(DPM_TRANSPORT_TYPE_T tDpmTransportType)
{
	DPM_HANDLE_T tHandle;
	HOSTDEF(ptAsicCtrlArea);
	BOOTING_T tResult;


	/* Can the DPM clocks be enabled? */
	if( (ptAsicCtrlArea->ulClock_enable_mask&HOSTMSK(clock_enable_dpm))==0 )
	{
		/* No, the clocks can not be enabled. */
		trace_message(TRACEMSG_DPM_DpmClocksMaskedOut);
		tResult = BOOTING_Not_Allowed;
	}
	else
	{
		/* Initialize the DPM. */
		dpm_init(&tHandle, tDpmTransportType);

		/* Boot from DPM. */
		tResult = boot_dpm_common(&tHandle);
	}

	return tResult;
}



BOOTING_T boot_idpm(void)
{
	HOSTDEF(ptAsicCtrlArea);
	DPM_HANDLE_T tHandle;
	int iIsLocked;
	BOOTING_T tResult;
	unsigned long ulValue;


	/* Booting from IDPM can only work if the ROM watchdog is still running. */
	iIsLocked = is_netx_locked();
	if( iIsLocked==0 )
	{
		/* The ROM watchdog is still running. */
		trace_message(TRACEMSG_DPM_IdpmCanNotRunWithWatchdog);
		tResult = BOOTING_Not_Allowed;
	}
	else
	{
		/* NOTE: The IDPM is completely internal. There is no need
		 *       to check if booting from this source is allowed.
		 */

		/* Initialize the DPM. */
		idpm0_init(&tHandle);

		/* Allow R/W accesses from the A9 to the handshake area 0. */
		ulValue  = HOSTMSK(firewall_cfg_netx_ramhs0_wp_cr7);
		ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_wp_ca9);
		ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_rp_cr7);
		ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_rp_ca9);
		__IRQ_LOCK__;
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
		ptAsicCtrlArea->aulFirewall_cfg_netx_ramhs[0] = ulValue;
		__IRQ_UNLOCK__;

		/* Boot from DPM. */
		tResult = boot_dpm_common(&tHandle);
	}

	return tResult;
}



BOOTING_T boot_pcie(void)
{
	HOSTDEF(ptAsicCtrlArea);
	DPM_HANDLE_T tHandle;
	BOOTING_T tResult;
	unsigned long ulValue;


	/* NOTE: The PCI express signals are dedicated pins. There is no need
	 *       to check if booting from this source is allowed.
	 */

	/* NOTE: The PCIe can not reach the IDPM because the firewall is closed while the ROM_WDG is still running.
	 *       It must use an area in the RAP RAM instead. This works quite well for booting, but it does not allow
	 *       the use of the reset register. During the ROM code this is no problem, but once the firmware is started,
	 *       all kinds of mayhem can happen. To allow a hardware reset in this stage, the IDPM registers are mapped
	 *       to the PCIE whenever a firmware is started.
	 */

	/* Initialize the DPM and open the firewall for IDPM0.
	 * NOTE: Although the firewall is configured here, the settings will
	 *       not become active until the ROM_WDG is stopped.
	 * NOTE: the handle will be initialized here for IDPM, but later
	 *       overwritten with the PCIe settings.
	 */
	idpm0_init(&tHandle);
	/* Set the ID to "stopped" as this is only visible when a firmware is started. */
	tHandle.ptDpmBlocks->tInformationBlock.ulDpmBootId = HBOOT_DPM_ID_STOPPED;
	memcpy(tHandle.ptDpmBlocks->tInformationBlock.acId, acId_RomCodeDpmStopped, sizeof(acId_RomCodeDpmStopped));

	/* Setup the PCIE core. */
	tResult = pcie_init(&tHandle);
	if( tResult==BOOTING_Ok )
	{
		/* Restart the transport channel in case PCIe was already configured. */
		transport_interface_dpm_restart(&tHandle);

		/* Allow R/W accesses from PCIe to the handshake area 0. */
		ulValue  = HOSTMSK(firewall_cfg_netx_ramhs0_wp_cr7);
		ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_wp_rest);
		ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_rp_cr7);
		ulValue |= HOSTMSK(firewall_cfg_netx_ramhs0_rp_rest);
		__IRQ_LOCK__;
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
		ptAsicCtrlArea->aulFirewall_cfg_netx_ramhs[0] = ulValue;
		__IRQ_UNLOCK__;

		/* Boot from DPM. */
		tResult = boot_dpm_common(&tHandle);
	}

	return tResult;
}


/*-----------------------------------*/
