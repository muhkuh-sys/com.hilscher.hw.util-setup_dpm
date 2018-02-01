
#include "portcontrol.h"

#include "netx_io_areas.h"
#include "options.h"



void portcontrol_init(void)
{
	size_t sizCfgCnt;
	size_t sizCfgEnd;
	unsigned long ulValue;
	volatile unsigned long *pulPortControl;


	pulPortControl = (volatile unsigned long*)HOSTADDR(PORTCONTROL);

	/* Loop over all entries in the options table. */
	sizCfgCnt = 0;
	sizCfgEnd = sizeof(g_t_romloader_options.tPortControlDefault.aus) / sizeof(g_t_romloader_options.tPortControlDefault.aus[0]);
	while( sizCfgCnt<sizCfgEnd )
	{
		ulValue = g_t_romloader_options.tPortControlDefault.aus[sizCfgCnt];
		if( ulValue!=PORTCONTROL_SKIP )
		{
			pulPortControl[sizCfgCnt] = ulValue;
		}
		++sizCfgCnt;
	}
}



void portcontrol_apply(const unsigned short *pusIndex, const unsigned short *pusConfiguration, size_t sizConfiguration)
{
	const unsigned short *pusIndexCnt;
	const unsigned short *pusIndexEnd;
	const unsigned short *pusConfigurationCnt;
	unsigned long ulConfiguration;
	unsigned long ulOffset;
	volatile unsigned long *pulPortControl;


	pulPortControl = (volatile unsigned long*)HOSTADDR(PORTCONTROL);

	pusIndexCnt = pusIndex;
	pusIndexEnd = pusIndex + sizConfiguration;
	pusConfigurationCnt = pusConfiguration;
	while( pusIndexCnt<pusIndexEnd )
	{
		/* Get the value. */
		ulOffset = (unsigned long)(*(pusIndexCnt++));
		ulConfiguration = (unsigned long)(*(pusConfigurationCnt++));

		if( ulConfiguration!=PORTCONTROL_SKIP && ulOffset!=PORTCONTROL_SKIP )
		{
			/* Write the configuration. */
			pulPortControl[ulOffset] = ulConfiguration;
		}
	}
}



void portcontrol_apply_mmio(const unsigned char *pucMmioIndex, const unsigned short *pusConfiguration, size_t sizConfiguration)
{
	const unsigned char *pucMmioIndexCnt;
	const unsigned char *pucMmioIndexEnd;
	const unsigned short *pusConfigurationCnt;
	unsigned long ulConfiguration;
	unsigned long ulOffset;
	volatile unsigned long *pulPortControl;


	pulPortControl = (volatile unsigned long*)HOSTADDR(PORTCONTROL);

	pucMmioIndexCnt = pucMmioIndex;
	pucMmioIndexEnd = pucMmioIndex + sizConfiguration;
	pusConfigurationCnt = pusConfiguration;
	while( pucMmioIndexCnt<pucMmioIndexEnd )
	{
		ulOffset = (unsigned long)(*(pucMmioIndexCnt++));
		ulConfiguration = (unsigned long)(*(pusConfigurationCnt++));

		if( ulOffset!=0xffU && ulConfiguration!=PORTCONTROL_SKIP )
		{
			/* MMIO0 is at 16,11.
			 * From MMIO1 on the pins start at 3,0 and continue sequentially. */
			if( ulOffset==0 )
			{
				ulOffset += PORTCONTROL_INDEX(16,11);
			}
			else
			{
				ulOffset += PORTCONTROL_INDEX( 3, 0);
			}

			/* Write the configuration. */
			pulPortControl[ulOffset] = ulConfiguration;
		}
	}
}



void portcontrol_restore(const unsigned short *pusIndex, size_t sizConfiguration)
{
	const unsigned short *pusIndexCnt;
	const unsigned short *pusIndexEnd;
	unsigned long ulConfiguration;
	unsigned long ulOffset;
	volatile unsigned long *pulPortControl;


	pulPortControl = (volatile unsigned long*)HOSTADDR(PORTCONTROL);

	pusIndexCnt = pusIndex;
	pusIndexEnd = pusIndex + sizConfiguration;
	while( pusIndexCnt<pusIndexEnd )
	{
		/* Get the value. */
		ulOffset = (unsigned long)(*(pusIndexCnt++));
		ulConfiguration = (unsigned long)(g_t_romloader_options.tPortControlDefault.aus[ulOffset]);

		if( ulConfiguration!=PORTCONTROL_SKIP && ulOffset!=PORTCONTROL_SKIP )
		{
			/* Write the configuration. */
			pulPortControl[ulOffset] = ulConfiguration;
		}
	}
}



void portcontrol_restore_mmio(const unsigned char *pucMmioIndex, size_t sizConfiguration)
{
	const unsigned char *pucMmioIndexCnt;
	const unsigned char *pucMmioIndexEnd;
	unsigned long ulConfiguration;
	unsigned long ulOffset;
	volatile unsigned long *pulPortControl;


	pulPortControl = (volatile unsigned long*)HOSTADDR(PORTCONTROL);

	pucMmioIndexCnt = pucMmioIndex;
	pucMmioIndexEnd = pucMmioIndex + sizConfiguration;
	while( pucMmioIndexCnt<pucMmioIndexEnd )
	{
		/* Get the value. */
		ulOffset = (unsigned long)(*(pucMmioIndexCnt++));
		if( ulOffset!=0xffU )
		{
			/* MMIO0 is at 16,11.
			 * From MMIO1 on the pins start at 3,0 and continue sequentially. */
			if( ulOffset==0 )
			{
				ulOffset += PORTCONTROL_INDEX(16,11);
			}
			else
			{
				ulOffset += PORTCONTROL_INDEX( 3, 0);
			}

			ulConfiguration = (unsigned long)(g_t_romloader_options.tPortControlDefault.aus[ulOffset]);
			if( ulConfiguration!=PORTCONTROL_SKIP )
			{
				/* Write the configuration. */
				pulPortControl[ulOffset] = ulConfiguration;
			}
		}
	}
}
