/***************************************************************************
 *   Copyright (C) 2005 - 2012 by Hilscher GmbH                            *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include <stddef.h>
#include "hboot_netx4000_trace_messages.h"


#ifndef __TRACE_H__
#define __TRACE_H__


void trace_init(void);

void trace_message(TRACEMSG_T tMsg);
void trace_message_uc(TRACEMSG_T tMsg, unsigned char ucData);
void trace_message_us(TRACEMSG_T tMsg, unsigned short usData);
void trace_message_ul(TRACEMSG_T tMsg, unsigned long ulData);
void trace_message_data(TRACEMSG_T tMsg, const void *pvData, size_t sizData);

void trace_write_restart_cookie(void);


#endif  /* __TRACE_H__ */

