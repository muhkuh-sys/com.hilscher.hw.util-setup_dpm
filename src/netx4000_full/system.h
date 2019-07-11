/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/

#include "asic_types.h"

#ifndef __SYSTEM_H__
#define __SYSTEM_H__


/*
 * check System configuration
 */
#ifndef ASIC_TYP
#       error "System not configured! Asic typ missing"
#endif

#if ASIC_TYP!=ASIC_TYP_NETX4000
#       error "Invalid Asic typ, must be 4000!"
#endif

#ifndef ASIC_ENV
#       error "System not configured! Asic env information missing"
#endif

#if ASIC_ENV!=ASIC_ENV_ASIC && ASIC_ENV!=ASIC_ENV_SCIT_BOARD && ASIC_ENV!=ASIC_ENV_SIMU
#       error "Unknown Asic env!"
#endif

#if !defined(CFG_ARTIFICIAL_KEYROM)
#       error "System not configured! KEYROM information missing. Please set CFG_ARTIFICIAL_KEYROM."
#endif

#if CFG_ARTIFICIAL_KEYROM!=0 && CFG_ARTIFICIAL_KEYROM!=1
#       error "Unknown value for CFG_ARTIFICIAL_KEYROM, must be 0 or 1."
#endif

#if !defined(CFG_ARTIFICIAL_OTP_FUSES)
#       error "System not configured! OTP_FUSES information missing. Please set CFG_ARTIFICIAL_OTP_FUSES."
#endif

#if CFG_ARTIFICIAL_OTP_FUSES!=0 && CFG_ARTIFICIAL_OTP_FUSES!=1
#       error "Unknown value for CFG_ARTIFICIAL_OTP_FUSES, must be 0 or 1."
#endif

/*-------------------------------------*/

/*
 * The clock settings depend on the environment.
 */

/* The netX area always runs at 100MHz. */
#define DEV_FREQUENCY 100000000L

/*-------------------------------------*/

/* bootloader version */
#define NETX_VERSION_STEP 'D'

/*
 * feature configuration for netX4000
 */
/* default value for io_config_mask */
#define HBOOT_DEFAULT_IO_CFG_MASK                HOSTDFLT(io_config_mask)
#define HBOOT_DEFAULT_IO_CFG2_MASK               HOSTDFLT(io_config2_mask)
/* default value for clk_enable_mask */
#define HBOOT_DEFAULT_CLK_ENABLE_MASK            HOSTDFLT(clock_enable_mask)

/*-------------------------------------*/

/* 'NXBL' DPM boot identifier ('NXBL') */
#define HBOOT_DPM_ID_LISTENING          0x4c42584e
#define HBOOT_DPM_ID_STOPPED            0x464e5552
/* DPM Bootloader version, counts up from 0xf0 */
#define HBOOT_DPM_BOOT_VERSION          0xf3
/* DPM Serial Console version, counts down from 0xfe */
#define HBOOT_DPM_COMM_VERSION          0xfc

/* Bits 10 - 31 of the netx_version register, which is mirrored to the DPM area.
 * There it is used to identify the netX.
 */
#define HBOOT_NETX4000FULL_DPM_VERSION_COOKIE         0x84524c00
#define HBOOT_NETX4000SMALL_DPM_VERSION_COOKIE        0x93615b00

/*-------------------------------------*/

#define HBOOT_NETX4000FULL_PCI_DEVICE_VENDOR          0x400015cf
#define HBOOT_NETX4000FULL_PCI_REVISION_CLASSCODE     0xff000001
#define HBOOT_NETX4000FULL_PCI_SUBDEVICE_SUBVENDOR    0x400015cf

#define HBOOT_NETX4000SMALL_PCI_DEVICE_VENDOR         0x410015cf
#define HBOOT_NETX4000SMALL_PCI_REVISION_CLASSCODE    0xff000001
#define HBOOT_NETX4000SMALL_PCI_SUBDEVICE_SUBVENDOR   0x410015cf


#define HBOOT_NETX4000FULL_USB_VENDOR_ID              0x1939U
#define HBOOT_NETX4000FULL_USB_DEVICE_ID              0x002aU
#define HBOOT_NETX4000FULL_USB_RELEASE_ID             0x0002U
#define HBOOT_NETX4000FULL_USB_DEVICE_STRING          {  8, 'n', 'e', 't', 'X', '4', '0', '0', '0',   0,   0,   0,   0,   0,   0,   0,   0 }

#define HBOOT_NETX4000SMALL_USB_VENDOR_ID             0x1939U
#define HBOOT_NETX4000SMALL_USB_DEVICE_ID             0x002bU
#define HBOOT_NETX4000SMALL_USB_RELEASE_ID            0x0002U
#define HBOOT_NETX4000SMALL_USB_DEVICE_STRING         {  8, 'n', 'e', 't', 'X', '4', '1', '0', '0',   0,   0,   0,   0,   0,   0,   0,   0 }


/*-------------------------------------*/

#endif  /* __SYSTEM_H__ */

