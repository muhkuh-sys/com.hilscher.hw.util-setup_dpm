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

#if ASIC_TYP!=ASIC_TYP_NETX4000_RELAXED
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
 * feature configuration
 */

#if ASIC_TYP==500
	/*
	 * feature configuration for netX500
	 */
#       if ASIC_ALLFEATURES==0
		/* default value for io_config_mask */
#               define HBOOT_DEFAULT_IO_CFG_MASK        0x1fffffff
		/* default value for clk_enable_mask */
#               define HBOOT_DEFAULT_CLK_ENABLE_MASK    0x000003ff
#       else
		/* default value for io_config_mask */
#               define HBOOT_DEFAULT_IO_CFG_MASK        0x9fffffff
		/* default value for clk_enable_mask */
#               define HBOOT_DEFAULT_CLK_ENABLE_MASK    0x000003ff
#       endif
#elif ASIC_TYP==100
	/*
	 * feature configuration for netX100
	 */
#       if ASIC_ALLFEATURES==0
		/* default value for io_config_mask */
#               define HBOOT_DEFAULT_IO_CFG_MASK        0x1ffffffc
		/* default value for clk_enable_mask */
#               define HBOOT_DEFAULT_CLK_ENABLE_MASK    0x000001ff
#       else
		/* default value for io_config_mask */
#               define HBOOT_DEFAULT_IO_CFG_MASK        0x9fffffff
		/* default value for clk_enable_mask */
#               define HBOOT_DEFAULT_CLK_ENABLE_MASK    0x000003ff
#       endif
#elif ASIC_TYP==56
	/*
	 * feature configuration for netX56
	 */
	/* default value for io_config_mask */
#       define HBOOT_DEFAULT_IO_CFG_MASK                0x0000ffff
#       define HBOOT_DEFAULT_IO_CFG2_MASK               0x00ffffff
	/* default value for clk_enable_mask */
#       define HBOOT_DEFAULT_CLK_ENABLE_MASK            0x000f3333
#elif ASIC_TYP==50
	/*
	 * feature configuration for netX50
	 */
#       if ASIC_ALLFEATURES==0
		/* default value for io_config_mask */
#               define HBOOT_DEFAULT_IO_CFG_MASK        0x608007ff
		/* default value for clk_enable_mask */
#               define HBOOT_DEFAULT_CLK_ENABLE_MASK    0x00000d33
#       else
		/* default value for io_config_mask */
#               define HBOOT_DEFAULT_IO_CFG_MASK        0xe08007ff
		/* default value for clk_enable_mask */
#               define HBOOT_DEFAULT_CLK_ENABLE_MASK    0x00000d33
#       endif
#elif ASIC_TYP==10
	/*
	 * feature configuration for netX10
	 */
	/* default value for io_config_mask */
#       define HBOOT_DEFAULT_IO_CFG_MASK                0x00bfffffU
	/* default value for clk_enable_mask */
#       define HBOOT_DEFAULT_CLK_ENABLE_MASK            0x00007511U
#elif ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
	/*
	 * feature configuration for netX4000
	 */
	/* default value for io_config_mask */
#       define HBOOT_DEFAULT_IO_CFG_MASK                HOSTDFLT(io_config_mask)
#       define HBOOT_DEFAULT_IO_CFG2_MASK               HOSTDFLT(io_config2_mask)
	/* default value for clk_enable_mask */
#       define HBOOT_DEFAULT_CLK_ENABLE_MASK            HOSTDFLT(clock_enable_mask)
#else
#       error "Missing feature configuration for current chip type!"
#endif

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
#define HBOOT_DPM_VERSION_COOKIE        0x73413c00

/*-------------------------------------*/

#define HBOOT_PCI_DEVICE_VENDOR         0x400015cf
#define HBOOT_PCI_REVISION_CLASSCODE    0xff000000

/*-------------------------------------*/

#endif  /* __SYSTEM_H__ */

