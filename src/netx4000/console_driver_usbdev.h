/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __CONSOLE_DRIVER_USBDEV_H__
#define __CONSOLE_DRIVER_USBDEV_H__

#include "console_io_interface.h"


typedef struct STRUCT_USB_CONFIGURATION
{
	unsigned short us_vendor_id;
	unsigned short us_device_id;
	unsigned short us_release_id;
	unsigned char uc_config_characteristics;
	unsigned char uc_maximum_power_consumption;
	unsigned char t_vendor_string[1+16];
	unsigned char t_device_string[1+16];
	unsigned char t_serial_string[1+16];
} USB_CONFIGURATION_T;


int console_driver_usbdev_init_sniff(CONSOLE_DEVICELIST_ENTRY_T *ptDevice);


#endif  /* __CONSOLE_DRIVER_USBDEV_H__ */

