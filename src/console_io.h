/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/

#ifndef __CONSOLE_IO_H__
#define __CONSOLE_IO_H__


#include <stdarg.h>

#include "console_io_interface.h"


#define CONSOLE_MAX_LINE_LENGTH 256

#define CONSOLE_ESC 0x1B
#define CONSOLE_BACKSPACE 0x7F


typedef enum ENUM_CONSOLE_KEYS
{
	CONSOLE_NONE		= -1,
	CONSOLE_CURSOR_UP	= -2,
	CONSOLE_CURSOR_DOWN	= -3,
	CONSOLE_CURSOR_LEFT	= -4,
	CONSOLE_CURSOR_RIGHT	= -5,
	CONSOLE_HOME		= -6,
	CONSOLE_END		= -7,
	CONSOLE_DELETE		= -8,
	CONSOLE_PAGE_UP		= -9,
	CONSOLE_PAGE_DOWN	= -10,
	CONSOLE_INSERT		= -11,
	CONSOLE_F1		= -12,
	CONSOLE_F2		= -13,
	CONSOLE_F3		= -14,
	CONSOLE_F4		= -15,
	CONSOLE_F5		= -16,
	CONSOLE_F6		= -17,
	CONSOLE_F7		= -18,
	CONSOLE_F8		= -19,
	CONSOLE_F9		= -20,
	CONSOLE_F10		= -21,
	CONSOLE_F11		= -22,
	CONSOLE_F12		= -23,
	CONSOLE_ENTER		= -24
} CONSOLE_KEYS_T;


typedef enum ENUM_CONSOLE_LINEFEED
{
	CONSOLE_LINEFEED_LF,		/* for Unix, Linux, Amiga, BeOS and MacOS X */
	CONSOLE_LINEFEED_CR,		/* for Apple II, old MacOS, OS-9 and C64 */
	CONSOLE_LINEFEED_CRLF		/* for Dos, Windows, CP/M, OS/2, and Symbian */
} CONSOLE_LINEFEED_T;


typedef enum ENUM_CONSOLE_DEVICE
{
	CONSOLE_DEVICE_NONE             = 0,
	CONSOLE_DEVICE_UART0            = 1,
	CONSOLE_DEVICE_UART1            = 2,
	CONSOLE_DEVICE_UART2            = 3,
	CONSOLE_DEVICE_USBDEV           = 4,
	CONSOLE_DEVICE_ETH              = 5,
	CONSOLE_DEVICE_DCC              = 6,
	CONSOLE_DEVICE_RAPUART0         = 7,
	CONSOLE_DEVICE_RAPUART1         = 8,
	CONSOLE_DEVICE_RAPUART2         = 9,
	CONSOLE_DEVICE_RAPUART3         = 10
	/* NOTE: Add an element here? Increase the define below! */
} CONSOLE_DEVICE_T;
#define __CONSOLE_DEVICE_MAX__ 11


typedef struct STRUCT_CONSOLE_IO_OPTIONS
{
	unsigned char uc_console_line_size;
	unsigned char uc_console_linefeed_mode;

	/* console devices */
	unsigned char auc_console_devices[4];

	/* UART diagnostic settings */
	unsigned char uc_uart_diagnostics_device;
} CONSOLE_IO_OPTIONS_T;


void console_io_init(void);
POLL_CONNECT_T console_io_poll(unsigned long ulBlinkMask, unsigned long ulBlinkState);

void console_init_diagnostic_device(void);

const char *console_io_read_line(unsigned int uiMaxLineSize);
const char *console_io_read_plain_line(unsigned int uiMaxLineSize);

unsigned char console_io_console_get(void);
void console_io_console_put(unsigned int uiChar);
unsigned int console_io_console_peek(void);
void console_io_console_flush(void);

void uprintf(const char *pcFmt, ...);


#endif	/* __CONSOLE_IO_H__ */
