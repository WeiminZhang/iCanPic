/*****************************************************************************
*	GSI a socket interface for Micro-controllers and Controller Area Network *
*   Copyright (C) 2014 Glenn Self                                            *
*                                                                            *
*   This program is free software: you can redistribute it and/or modify     *
*   it under the terms of the GNU General Public License as published by     *
*   the Free Software Foundation, either version 3 of the License, or        *
*   (at your option) any later version.                                      *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
*****************************************************************************/
/*
ERROR constants
This file though generic contains error codes for:
SR,PDD;
TIPS;
*/
#ifndef GERROR_H_INCLUDED
#define GERROR_H_INCLUDED

/* Don't forget to update gexcpt.cpp with error text for these errors*/

enum actionType{
actionEsc									=0,
actionIgnore								=1,
actionRetry									=2,
actionAbort									=3,
actionTerminate							=4,
actionThrow									=5,
actionFail									=6
};

/*This enum has corresponding EQU in srerror.inc */

enum errorType{
errNone										=0,
						
/*These are errno values from the C library */

errDom										=1,	/* domain error */
errRange    	   						=2,	/* range error */
errBadMode        						=3,
errBadName        						=4,
errIsTempMem      						=5,
errBadShare       						=6,
errBufMode        						=7,
errErrSet         						=8,
errIsOpen         						=9,
errNotExist      							=10,
errNotInit       							=11,
errNullFcb       							=12,
errOutOfMem      							=13,
errSmallBf       							=14,
errExist         							=16,
errNoGen         							=17,
errNoSeek        							=19,
errBadPos        							=20,
errBadSeek       							=22,
errNoEnt         							=23,
errAccess        							=24,
errMFile         							=25,
errNoCmd         							=26,
errGetAndPut     							=28,
errPastEof       							=29,
errNotRead       							=30,
errTooManyUnGetc 							=31,
errUnGetEof      							=32,
errPutUnGet      							=33,
errChild         							=34,
errIntr          							=35,
errInVal         							=36,
errNoExec        							=37,
errAgain         							=38,
errBadType       							=39,
errNotWrite      							=40,
errPutAndGet     							=41,
errLargeBf       							=42,
errBadF          							=43,
errXDev          							=44,
errNoSpc         							=45,
errMath          							=46,
errModName       							=47,
errMaxAttr       							=49,
errReadError     							=50,
errBadAttr       							=51,
errOs2Err		  							=60,	/* OS/2 error*/
errDriveLocked								=108,
/*End of C library error codes */

errFileCreate								=301,
errFileDelete								=302,
errFileOpen									=303,
errFileClose								=304,
errFileRead									=305,
errFileWrite								=306,
errFileSeek									=307,
errFileExists								=308,
errFileNotFound							=309,
errFileBadDrive							=310,
errFileBadPath								=311,
errFileBadFName							=312,
errFileBadExt								=313,
errStream									=314,

errFilterInsuffData						=320,
errNotSupported							=330,

errFPE										=341,

errBadId                                    =342,
errBadAddress								=343,
errBadPort									=344,

errMem										=350,
errNew										=351,
errDelete									=352,
errNoAlloc									=353,
errOutOfResource							=360,
errTimeout									=361,
errBounds									=362,
errEmpty                                    =363,
errParameter                                =364,
errEcho                                     =365,

errGSIAck                                   =366,
errGSIRepeat                                =367,
errGSIError                                 =368,
errGSIFatal                                 =369,
errGSIFunctionFail                          =370,				/**/
errGSIReSync                                =371,
errGSISlaveReq                              =372,               //node has requested that this code become slave
errGSIFail                                  =373,               //differentiate from generic errFail
errGSIReset                                 =374,               //microcontroller has reset etc.
errFail								        =400,    
errNotImplemented							=401,
errUserCancel								=402,
errChecksum									=403,

errSocket                                   =410,
errSocketLost                               =411,
errSocketRead                               =412,
errSocketWrite                              =413,
errSocketInvalid							=414,
errSocketNoHost								=415,	

errBadHandle								=501,
errAlreadyOpen                              =502,
errNotOpen                                  =503,
errBadDevice								=504,
errNullStr									=510,

errBusy										=511,	/**/
errWaiting									=512,	/**/
errIO										=513,
errWouldBlock								=514,

errNoADCBoard	  							=550,

errPDDFunc									=600,
errPDDParam									=601,
errPDDVerify								=602,
errPDDNotSupported						=603,
errPDDMemAlreadyAllocated     		    =604,
errPDDMemNotAllocated           		=605,
errPDDNoBoardSelected					=606,


errADCRead									=700,

errADCISR									=701,
errADCChanOff								=702,

errVDDRead	=720,		/*similar to errADCRead*/

errDMABusy									=750,				    

errQueueWrite								=800,
errQueueEmpty								=801,
errQueueNotEmpty							=802,
errQueueOverflow							=803,

errSpiOvr									=850,	//spi overrun
errSpiModeFault								=851,

errOvr										=860,	//overrun
errNoiseError								=862,
errFrame									=863,
errParity									=864,

errVIO										=900,
errKBD										=1000,

errFatal										=9000,
errUnknown									=9999,
errError										=10000
};

#endif  /* GERRORH_INCLUDED */

