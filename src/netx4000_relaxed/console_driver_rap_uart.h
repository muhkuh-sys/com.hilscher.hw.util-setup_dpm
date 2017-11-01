/*---------------------------------------------------------------------------
  Author : Christoph Thelen

           Hilscher GmbH, Copyright (c) 2015, All Rights Reserved

           Redistribution or unauthorized use without expressed written 
           agreement from the Hilscher GmbH is forbidden
---------------------------------------------------------------------------*/


#ifndef __CONSOLE_DRIVER_RAP_UART_H__
#define __CONSOLE_DRIVER_RAP_UART_H__

#include "console_io_interface.h"
#include "system.h"

#if 0
typedef enum ENUM_RAP_UART_BAUDRATE
{
	RAP_UART_BAUDRATE_300    =    3,
	RAP_UART_BAUDRATE_600    =    6,
	RAP_UART_BAUDRATE_1200   =   12,
	RAP_UART_BAUDRATE_2400   =   24,
	RAP_UART_BAUDRATE_4800   =   48,
	RAP_UART_BAUDRATE_9600   =   96,
	RAP_UART_BAUDRATE_19200  =  192,
	RAP_UART_BAUDRATE_38400  =  384,
	RAP_UART_BAUDRATE_57600  =  576,
	RAP_UART_BAUDRATE_115200 = 1152
} RAP_UART_BAUDRATE_T;
#endif

/* The RAP UART seems to run with a clock of f_U = 100MHz
 *
 * The baud divider is then:
 *   bauddiv = f_U / (16*baudrate)
 *
 * For a baud rate of 115200 the result for bauddiv is:
 *   bauddiv = 100000000 / (16*115200)
 *           = 54,253472222222
 * The integer part is simply int(bauddiv), so in this case it is 54.
 * The fractional part is int((0.253472222222×64)+0.5) = 16.
 *
 * So the complete baud divider is 54 + (16/64) = 54,25 .
 *
 *
 * On the SCIT board the frequency is currently 10MHz.
 * So bauddiv is here:
 *   bauddiv = 10000000 / (16*115200)
 *           = 5,4253472222222
 * The integer part is 5.
 * The fractional part is int((0.4253472222222*64)+0.5) = 27.
 *
 * So the complete baud divider is 5 + (27/64) = 5,421875 .
 */

#if ASIC_ENV==ASIC_ENV_SCIT_BOARD
#       define RAP_UART_BAUDRATE_DIV_115200 0x0005001b
#else
#       define RAP_UART_BAUDRATE_DIV_115200 0x00360010
#endif


/* DEV_BAUDRATE is 100 times to small -> multiply with 100 (or divide by DEV_FREQUENCY/100) */
#define RAP_UART_BAUDRATE_DIV(a) ((a*16*65536)/(DEV_FREQUENCY/100))


typedef enum RAP_UART_MODE_BYTESIZE_ENUM
{
	RAP_UART_MODE_BYTESIZE_5BIT = 0x00,
	RAP_UART_MODE_BYTESIZE_6BIT = 0x20,
	RAP_UART_MODE_BYTESIZE_7BIT = 0x40,
	RAP_UART_MODE_BYTESIZE_8BIT = 0x60
} RAP_UART_MODE_BYTESIZE_T;

/* #define UART_MODE_ */
#define RAP_UART_MODE_2STOPBITS       0x08
#define RAP_UART_MODE_EVEN_PARITY     0x04
#define RAP_UART_MODE_PARITY_ENABLE   0x02
#define RAP_UART_MODE_ENABLE_RTSCTS   0x01


typedef struct RAP_UART_CONFIGURATION_STRUCT
{
	unsigned long ulBaudDiv;
	unsigned char uc_mode;
	unsigned short ausPortControl[4];
} RAP_UART_CONFIGURATION_T;


int rap_uart_init_sniff(CONSOLE_DEVICELIST_ENTRY_T *ptDevice, unsigned int uiUartUnit);


#endif  /* __CONSOLE_DRIVER_RAP_UART_H__ */
