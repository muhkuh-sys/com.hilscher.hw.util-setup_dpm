#ifndef __BOOT_SDIO_H__
#define __BOOT_SDIO_H__

#include "boot_common.h"


typedef struct SDIO_BOOT_OPTIONS_STRUCT
{
	unsigned long   ulPowerUpTimeoutTicks;
	unsigned long   ulResetDelayTicks;
	unsigned long   ulInitialSpeedKHz;
	unsigned long   ulMaximumSpeedKHz;
	unsigned long   ulInitialClockGenerationUs;
	unsigned short ausPortControl[8];
	unsigned char  atFileName[12+1];
	unsigned char   ucHwTimeoutExponentInitialization;
	unsigned char   ucHwTimeoutExponentTransfer;
} SDIO_BOOT_OPTIONS_T;



BOOTING_T boot_sdio(void);


#endif  /* __BOOT_SDIO_H__ */

