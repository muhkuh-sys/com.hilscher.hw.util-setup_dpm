/*---------------------------------------------------------------------------
  Author : Christoph Thelen

           Hilscher GmbH, Copyright (c) 2006, All Rights Reserved

           Redistribution or unauthorized use without expressed written 
           agreement from the Hilscher GmbH is forbidden
---------------------------------------------------------------------------*/


#ifndef __CONSOLE_DRIVER_UART_H__
#define __CONSOLE_DRIVER_UART_H__

#include "console_io_interface.h"
#include "system.h"


typedef enum ENUM_UART_BAUDRATE
{
	UART_BAUDRATE_300    =    3,
	UART_BAUDRATE_600    =    6,
	UART_BAUDRATE_1200   =   12,
	UART_BAUDRATE_2400   =   24,
	UART_BAUDRATE_4800   =   48,
	UART_BAUDRATE_9600   =   96,
	UART_BAUDRATE_19200  =  192,
	UART_BAUDRATE_38400  =  384,
	UART_BAUDRATE_57600  =  576,
	UART_BAUDRATE_115200 = 1152
} UART_BAUDRATE_T;

/* DEV_BAUDRATE is 100 times to small -> multiply with 100 (or divide by DEV_FREQUENCY/100) */
#define UART_BAUDRATE_DIV(a) ((a*16*65536)/(DEV_FREQUENCY/100))


typedef enum UART_MODE_BYTESIZE_ENUM
{
	UART_MODE_BYTESIZE_5BIT = 0x00,
	UART_MODE_BYTESIZE_6BIT = 0x20,
	UART_MODE_BYTESIZE_7BIT = 0x40,
	UART_MODE_BYTESIZE_8BIT = 0x60
} UART_MODE_BYTESIZE_T;

/* #define UART_MODE_ */
#define UART_MODE_2STOPBITS		0x08
#define UART_MODE_EVEN_PARITY		0x04
#define UART_MODE_PARITY_ENABLE		0x02
#define UART_MODE_ENABLE_RTSCTS		0x01


typedef struct UART_CONFIGURATION_MMIOS_STRUCT
{
	unsigned char uc_rx_mmio;
	unsigned char uc_tx_mmio;
	unsigned char uc_rts_mmio;
	unsigned char uc_cts_mmio;
} UART_CONFIGURATION_MMIOS_T;


typedef union UART_CONFIGURATION_MMIO_ACCESS_UNION
{
	unsigned char auc[4];
	UART_CONFIGURATION_MMIOS_T s;
} UART_CONFIGURATION_MMIO_ACCESS_T;


typedef struct UART_CONFIGURATION_STRUCT
{
	UART_CONFIGURATION_MMIO_ACCESS_T uMmio;
	unsigned char uc_mode;
	unsigned short us_baud_div;
	unsigned char uc_connect1;
	unsigned char uc_connect2;
	unsigned short ausPortControl[4];
} UART_CONFIGURATION_T;


int uart_init_sniff(CONSOLE_DEVICELIST_ENTRY_T *ptDevice, unsigned int uiUartUnit);


#endif  /* __CONSOLE_DRIVER_UART_H__ */
