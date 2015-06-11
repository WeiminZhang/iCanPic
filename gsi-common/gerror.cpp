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
#include <stdint.h>
#include "wx/wx.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__


// for all others, include the necessary headers
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#ifdef __VISUALC__
#pragma warning(push, 3)
#endif

//std headers in here

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif


#include <gerror.hpp>


#ifdef __VISUALC__
#pragma warning(push, 1)
#endif

//std headers in here

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif


#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif



/*-------------------------------------------
        ctor
    ErrorClass::ErrorClass()
--------------------------------------------*/
ErrorClass::ErrorClass()
{
    m_ErrString.Empty();
	m_err=errError;
}

/*-------------------------------------------
        ctor
    ErrorClass::ErrorClass(errorType err)
--------------------------------------------*/

ErrorClass::ErrorClass(errorType err)
{
    m_TempErrString.Empty();
	m_err=err;
    m_ErrString=GetString(err);

}

/*-------------------------------------------
        dtor
    ErrorClass::~ErrorClass()
--------------------------------------------*/

ErrorClass::~ErrorClass()
{

}



/*---------------------------------------------
    wxString & ErrorClass::GetString()
---------------------------------------------*/
const wxString & ErrorClass::GetString()
{
    return m_ErrString;
}

/*---------------------------------------------
    wxString & ErrorClass::GetString(errorType err)
---------------------------------------------*/

const wxString & ErrorClass::GetString(errorType err)
{
    m_TempErrString.Empty();
    switch(err)
	{
    case errNone:
        m_TempErrString="No error";
        break;
    case errDom:
        m_TempErrString="Bad domain";
        break;
    case errRange:
        m_TempErrString="Bad range";
        break;
    case errBadMode:
        m_TempErrString="Bad Mode";
        break;
	case errBadName:
        m_TempErrString="Bad name";
        break;
	case errIsTempMem:
        m_TempErrString="Temporary memory";
        break;
	case errBadShare:
        m_TempErrString="Bad share";
		break;
	case errBufMode:
        m_TempErrString="Buff mode";        //
        break;
	case errErrSet:
        m_TempErrString="Set";
        break;
	case errIsOpen:
        m_TempErrString="Already open";
        break;
	case errNotExist:
        m_TempErrString="Doesn't exist";
        break;
	case errNotInit:
        m_TempErrString="Not initialized";
        break;
	case errNullFcb:
        m_TempErrString="Null FCB";
        break;
	case errOutOfMem:
        m_TempErrString="Out of memory";
        break;
	case errSmallBf:
        m_TempErrString="Small buffer";     //what is this?
        break;
	case errExist:
        m_TempErrString="Already exists";
        break;
	case errNoGen:
        m_TempErrString="No gen";       //what is this?
        break;
	case errNoSeek:
        m_TempErrString="Couldn't seek";
        break;
	case errBadPos:
        m_TempErrString="Bad position";
        break;
	case errBadSeek:
        m_TempErrString="Bad seek";
        break;
	case errNoEnt:
        m_TempErrString="No entry";
		break;
	case errAccess:
        m_TempErrString="Access";
        break;
	case errMFile:
        m_TempErrString="Memory file";
        break;
	case errNoCmd:
        m_TempErrString="No command";
        break;
	case errGetAndPut:
        m_TempErrString="Get and put";
        break;
	case errPastEof:
        m_TempErrString="Past end of file";
        break;
	case errNotRead:
        m_TempErrString="Not read";
        break;
    case errTooManyUnGetc:
        m_TempErrString="Too many ungetc";
		break;
	case errUnGetEof:
        m_TempErrString="Unget eof";
		break;
    case errPutUnGet:
        m_TempErrString="Put unget";
		break;
    case errChild:
        m_TempErrString="Child";
		break;
    case errIntr:
        m_TempErrString="Intr";     //what is this
		break;
    case errInVal:
        m_TempErrString="In val";   //what is this
		break;
    case errNoExec:
        m_TempErrString="No Exec";
		break;
    case errAgain:
        m_TempErrString="Again";	//what is this
		break;
    case errBadType:
        m_TempErrString="Bad type";
		break;
    case errNotWrite:
        m_TempErrString="Not written";
		break;
    case errPutAndGet:
        m_TempErrString="Put and get";
		break;
    case errLargeBf:
        m_TempErrString="Large buff";       //what is this
		break;
    case errBadF:
        m_TempErrString="Bad file";
		break;
	case errXDev:
        m_TempErrString="X device";         //what?
		break;
    case errNoSpc:
        m_TempErrString="No space";         //??
		break;
    case errMath:
        m_TempErrString="Math";
		break;
	case errModName:
        m_TempErrString="Moudle name";  //??
		break;
    case errMaxAttr:
        m_TempErrString="Max attrbute"; //??
		break;
    case errReadError:
        m_TempErrString="Read";
		break;
    case errBadAttr:
        m_TempErrString="Bad attribute";
		break;
    case errOs2Err:
        m_TempErrString="OS2";
		break;
	case errDriveLocked:
        m_TempErrString="Drive locked";
		break;
    case errFileCreate:
        m_TempErrString="File create";
		break;
    case errFileDelete:
        m_TempErrString="File delete";
		break;
    case errFileOpen:
        m_TempErrString="File open";
		break;
    case errFileClose:
        m_TempErrString="File close";
		break;
    case errFileRead:
        m_TempErrString="File read";
		break;
    case errFileWrite:
        m_TempErrString="File write";
		break;
    case errFileSeek:
        m_TempErrString="File seek";
		break;
    case errFileExists:
        m_TempErrString="File exists";
		break;
    case errFileNotFound:
        m_TempErrString="File not found";
		break;
    case errFileBadDrive:
        m_TempErrString="Bad drive";
		break;
    case errFileBadPath:
        m_TempErrString="Bad path";
		break;
    case errFileBadFName:
        m_TempErrString="Bad name";
		break;
    case errFileBadExt:
        m_TempErrString="Bad extension";
		break;
    case errStream:
        m_TempErrString="Stream";
		break;

    case errFilterInsuffData:
        m_TempErrString="Insufficent data";     //sr
		break;
    case errNotSupported:
        m_TempErrString="Not supported";
		break;

    case errFPE:
        m_TempErrString="Floating point";
		break;

	case errBadId:
        m_TempErrString="Bad id";
		break;
	case errBadAddress:
        m_TempErrString="Bad address";
		break;
	case errBadPort:
        m_TempErrString="Bad port";
		break;

	case errMem:
        m_TempErrString="Memory";
		break;
    case errNew:
        m_TempErrString="New";
		break;

    case errDelete:
        m_TempErrString="Delete";
		break;
    case errNoAlloc:
        m_TempErrString="Not allocated";
		break;
    case errOutOfResource:
        m_TempErrString="Out of resource";
		break;
    case errTimeout:
        m_TempErrString="Timeout";
		break;
    case errBounds:
        m_TempErrString="Bounds";
		break;
    case errEmpty:
        m_TempErrString="Empty";
		break;
	case errParameter:
        m_TempErrString="Parameter";
		break;
    case errEcho:
        m_TempErrString="Bad echo";
	    break;
//GSI microcontroller/CAN interface errors
    case errGSIAck:
        m_TempErrString="GSI acknowledge";
		break;
    case errGSIRepeat:
        m_TempErrString="GSI Repeat";
		break;
    case errGSIError:
        m_TempErrString="GSI";
		break;
    case errGSIFatal:
        m_TempErrString="GSI fatal";
		break;
    case errGSIFunctionFail:
        m_TempErrString="GSI function fail";
		break;
	case errGSIReSync:
        m_TempErrString="GSI resync";
		break;
    case errGSISlaveReq:
        m_TempErrString="GSI slave request";    //this is actually not an error but is returned on a re-sync
		break;
    case errGSIFail:
        m_TempErrString="GSI fail";
		break;
    case errGSIReset:
        m_TempErrString="GSI reset";            //mc has reset etc.
        break;
    case errFail:
        m_TempErrString="Fail";
		break;
    case errNotImplemented:
        m_TempErrString="Not implemented";
        break;
	case errUserCancel:
		m_TempErrString="User cancelled";
		break;
    case errSocket:
        m_TempErrString="Socket";
        break;
    case errSocketLost:
        m_TempErrString="Socket lost";
        break;
    case errSocketRead:
        m_TempErrString="Socket read";
        break;
    case errSocketWrite:
        m_TempErrString="Socket write";
        break;
    case errSocketInvalid:
        m_TempErrString="Socket Invalid";
        break;
    case errSocketNoHost:
        m_TempErrString="Socket no host";
        break;

    case errBadHandle:
        m_TempErrString="Bad handle";
		break;
    case errAlreadyOpen:                    //=502
        m_TempErrString="Already open";
		break;
    case errNotOpen:                        // 503,
        m_TempErrString="Not open";
		break;
	case errBadDevice:				//504
        m_TempErrString="Bad device";
		break;

    case errNullStr:
        m_TempErrString="Null string";
		break;

    case errBusy:
        m_TempErrString="Temp string";
		break;
    case errWaiting:
        m_TempErrString="Waiting";
		break;
    case errIO:
	    m_TempErrString="I/O";
		break;
    case errWouldBlock:
	    m_TempErrString="Would block";
		break;

    //GS os/2 physical device driver errors
    case errNoADCBoard:
        m_TempErrString="No ADC board";
    	break;
    case errPDDFunc:
        m_TempErrString="Bad PDD function";     //Physical device driver
		break;
    case errPDDParam:
        m_TempErrString="Bad PDD parameter";
		break;
    case errPDDVerify:
        m_TempErrString="PDD verify";
		break;
    case errPDDNotSupported:
        m_TempErrString="PDD not supported";
		break;
    case errPDDMemAlreadyAllocated:
        m_TempErrString="PDD memory already allocated";
		break;
    case errPDDMemNotAllocated:
        m_TempErrString="PDD memory not allocated";
		break;
    case errPDDNoBoardSelected:
        m_TempErrString="PDD no board selected";
		break;


    case errADCRead:
        m_TempErrString="ADC read";
		break;

    case errADCISR:
        m_TempErrString="ADC ISR error";        //interrupt service routine
		break;
    case errADCChanOff:
        m_TempErrString="ADC channel off";
		break;

    case errVDDRead:
        m_TempErrString="VDD read";	            //Virtual device driver
		break;

	case errDMABusy:
        m_TempErrString="DMA busy";
		break;
//end of GS os/2 pdd/vdd stuff

    case errQueueWrite:
        m_TempErrString="Queue write";
		break;
    case errQueueEmpty:
        m_TempErrString="Queue empty";
		break;
    case errQueueOverflow:
        m_TempErrString="Queue overflow";
		break;
//more os/2 stuff
    case errVIO:
        m_TempErrString="VIO";
		break;
    case errKBD:
        m_TempErrString="Keyboard";
		break;
//other generic errors
    case errFatal:
        m_TempErrString="Fatal";
		break;
	case errUnknown:
		m_TempErrString="Unknown";
		break;							//we found an error but couldn't identify it
    case errError:
        m_TempErrString="General";      //non-specific/undefined error
		break;
	default:							//caller has sent an invalid err code
        m_TempErrString="Undefined error code";      //non-specific/undefined error
		break;
	}
    m_TempErrString.Append(" error");
    return m_TempErrString;
}



