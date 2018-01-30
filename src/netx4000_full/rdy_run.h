/***************************************************************************
 *   Copyright (C) 2012 by Christoph Thelen                                *
 *   doc_bacardi@users.sourceforge.net                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef __RDY_RUN_H__
#define __RDY_RUN_H__

/*
 *  |m|s|
 *  |a|t|
 *  |s|a|
 *  |k|t|
 *  | |e|
 *  +-+-+----------------------------
 *  |0|0| led off
 *  +-+-+----------------------------
 *  |1|0| yellow
 *  +-+-+----------------------------
 *  |0|1| end of sequence (restart)
 *  +-+-+----------------------------
 *  |1|1| green
 *  +-+-+----------------------------
 */

/*                            *__YY  */
#define BLINKI_M_CONSOLE                            0x00000003
#define BLINKI_S_CONSOLE                            0x00000010

/*                *_______Y__YYYYYY  */
#define BLINKI_M_CONSOLE_ERROR                      0x0000010f
#define BLINKI_S_CONSOLE_ERROR                      0x00010000

/*                               *_G */
#define BLINKI_M_ETH_BOOTING                        0x00000001
#define BLINKI_S_ETH_BOOTING                        0x00000005

/*  _______Y___Y___Y___Y___Y__YYYYYY */
#define BLINKI_M_CONSOLE_OTP_ERROR                  0x0111113f
#define BLINKI_S_CONSOLE_OTP_ERROR                  0x00000000


/*-------------------------------------------------------------------------*/

/* SYST is the "Slow Yellow Story Teller". It shows long blinking sequences
 * the yellow "RDY" LED.
 *
 * The blink sequences are a combination of long and short signals. A long
 * signal is __YYYYYY and a short signal is ___Y. A pause is ____.
 *
 *  |m|s|
 *  |a|t|
 *  |s|a|
 *  |k|t|
 *  | |e|
 *  +-+-+----------------------------
 *  |0|0| off
 *  +-+-+----------------------------
 *  |1|0| long
 *  +-+-+----------------------------
 *  |0|1| end of sequence (restart)
 *  +-+-+----------------------------
 *  |1|1| short
 *  +-+-+----------------------------
 */

/*  LSS_*  */
#define SYST_M_DPM_WAIT_FOR_CONNECTION         0x00000007
#define SYST_S_DPM_WAIT_FOR_CONNECTION         0x00000016

/*  LSSS_*  */
#define SYST_M_ETH_MACHINE_INTERFACE           0x0000000f
#define SYST_S_ETH_MACHINE_INTERFACE           0x0000002e

/*  LSSSS_*  */
#define SYST_M_ETH_WAIT_FOR_LINK_UP            0x0000001f
#define SYST_S_ETH_WAIT_FOR_LINK_UP            0x0000005e

/*  LSSSSS_*  */
#define SYST_M_CONSOLE_OTP_ERROR               0x0000003f
#define SYST_S_CONSOLE_OTP_ERROR               0x000000be

/*  LLSS_*  */
#define SYST_M_SECURE_DPM_WAIT_FOR_CONNECTION  0x0000000f
#define SYST_S_SECURE_DPM_WAIT_FOR_CONNECTION  0x0000002c

/*  LLSSS_*  */
#define SYST_M_SECURE_ETH_MACHINE_INTERFACE    0x0000001f
#define SYST_S_SECURE_ETH_MACHINE_INTERFACE    0x0000005c

/*  LLSSSS_*  */
#define SYST_M_SECURE_ETH_WAIT_FOR_LINK_UP     0x0000003f
#define SYST_S_SECURE_ETH_WAIT_FOR_LINK_UP     0x000000bc

/*  LLSSSSS_*  */
#define SYST_M_SECURE_CONSOLE_OTP_ERROR        0x0000007f
#define SYST_S_SECURE_CONSOLE_OTP_ERROR        0x0000017c

/*-----------------------------------*/

typedef enum RDYRUN_ENUM
{
	RDYRUN_OFF      = 0,
	RDYRUN_GREEN    = 1,
	RDYRUN_YELLOW   = 2
} RDYRUN_T;


typedef struct BLINKI_HANDLE_STRUCT
{
	unsigned long long ullTimer;
	unsigned int uiCnt;
	unsigned long ulMask;
	unsigned long ulState;
} BLINKI_HANDLE_T;




typedef enum SYST_STATE_ENUM
{
	SYST_STATE_GetNextCmd   = 0,
	SYST_STATE_PhaseOn      = 1,
	SYST_STATE_PhaseOff     = 2
} SYST_STATE_T;


typedef struct SYST_HANDLE_STUCT
{
	unsigned long long ullTimer;
	SYST_STATE_T tState;
	unsigned long ulCurrentTicksPhaseOn;
	unsigned long ulCurrentTicksPhaseOff;
	unsigned int uiCnt;
	unsigned long ulMask;
	unsigned long ulState;
} SYST_HANDLE_T;


void rdy_run_setLEDs(RDYRUN_T tState);
void rdy_run_blinki_init(BLINKI_HANDLE_T *ptHandle, unsigned long ulMask, unsigned long ulState);
void rdy_run_blinki(BLINKI_HANDLE_T *ptHandle);
void rdy_run_syst_init(SYST_HANDLE_T *ptHandle, unsigned long ulMask, unsigned long ulState);
void rdy_run_syst(SYST_HANDLE_T *ptHandle);

/*-----------------------------------*/

#endif  /* __RDY_RUN_H__ */

