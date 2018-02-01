/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __CONSOLE_IO_INTERFACE_H__
#define __CONSOLE_IO_INTERFACE_H__


#include <stddef.h>

#include "cr7_global_timer.h"
#include "netx_io_areas.h"
#include "networking/network_interface.h"
#include "networking/stack/udp.h"


typedef enum CONSOLE_ETH_STATE_ENUM
{
	CONSOLE_ETH_STATE_NoLink             = 0,
	CONSOLE_ETH_STATE_LinkUp_Delay       = 1,
	CONSOLE_ETH_STATE_LinkUp_Ready       = 2,
	CONSOLE_ETH_STATE_Dhcp               = 3,
	CONSOLE_ETH_STATE_Ready              = 4,
	CONSOLE_ETH_STATE_WaitForConnection  = 5,
	CONSOLE_ETH_STATE_Connected          = 6,
	CONSOLE_ETH_STATE_Error              = 7
} CONSOLE_ETH_STATE_T;



typedef union CONSOLE_DEVICE_SPECIFIC_DATA_UNION
{
	struct CONSOLE_DEVICE_SPECIFIC_DATA_UART_STRUCT
	{
		HOSTADEF(UART) *ptUartArea;
		unsigned int uiUnit;
		size_t sizFill;
		unsigned char aucBuffer[5];
	} tUart;

	struct CONSOLE_DEVICE_SPECIFIC_DATA_USB_STRUCT
	{
		size_t sizFill;
		unsigned char aucBuffer[5];
	} tUsb;

	struct CONSOLE_DEVICE_SPECIFIC_DATA_ETH_STRUCT
	{
		const NETWORK_IF_T *ptNetworkIf;
		void *pvUser;
		CONSOLE_ETH_STATE_T tState;
		TIMER_HANDLE_T tLinkUpTimer;
		TIMER_HANDLE_T tEthernetHandlerTimer;
		UDP_ASSOCIATION_T *ptMiUdpAssoc;
		void *pvFirstUnicastPacket;
	} tEth;

	struct CONSOLE_DEVICE_SPECIFIC_DATA_RAP_UART_STRUCT
	{
		HOSTADEF(S_RAP_UART) *ptRapUartArea;
		unsigned int uiUnit;
		size_t sizFill;
		unsigned char aucBuffer[5];
	} tRapUart;
} CONSOLE_DEVICE_SPECIFIC_DATA_T;

typedef enum POLL_CONNECT_ENUM
{
	POLL_CONNECT_IDLE               = 0,
	POLL_CONNECT_CONSOLE            = 1,
	POLL_CONNECT_MACHINE_INTERFACE  = 2
} POLL_CONNECT_T;

typedef POLL_CONNECT_T (*CONSOLE_POLL_CONNECT)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData);
typedef void (*CONSOLE_INIT_TRANSFER)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData);
typedef unsigned char (*CONSOLE_GET_PFN)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData);
typedef void (*CONSOLE_PUT_PFN)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData, unsigned char ucChar);
typedef unsigned int (*CONSOLE_PEEK_PFN)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData);
typedef void (*CONSOLE_FLUSH_PFN)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData);
typedef void (*CONSOLE_CLOSE_PFN)(CONSOLE_DEVICE_SPECIFIC_DATA_T *ptData);

typedef struct CONSOLE_DEVICELIST_ENTRY_STRUCT
{
	int f_is_configured;
	CONSOLE_DEVICE_SPECIFIC_DATA_T t_device_specific_data;
	CONSOLE_POLL_CONNECT pfnPollConnect;
	CONSOLE_INIT_TRANSFER pfnInitTransfer;
	CONSOLE_GET_PFN pfnGet;
	CONSOLE_PUT_PFN pfnPut;
	CONSOLE_PEEK_PFN pfnPeek;
	CONSOLE_FLUSH_PFN pfnFlush;
	CONSOLE_CLOSE_PFN pfnClose;
} CONSOLE_DEVICELIST_ENTRY_T;


#endif	/* __CONSOLE_IO_INTERFACE_H__ */

