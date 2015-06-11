
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

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#include "wx/socket.h"

//#include "include/gsi.hpp"
#include "include/record.hpp"
#include "include/socket.hpp"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif
#if __GNUC__
    #define DEBUG_NEW new
#endif

DEFINE_EVENT_TYPE(wxEVT_GSI_SOCKET_COMMAND)
DEFINE_EVENT_TYPE(wxEVT_GSI_SOCKET_RECORD_RECEIVED)
IMPLEMENT_DYNAMIC_CLASS(wxGSISocketCommandEvent,wxNotifyEvent);
IMPLEMENT_DYNAMIC_CLASS(wxGSISocketRecordReceivedEvent,wxNotifyEvent);

/*****************************************************************************
			GSISocketData
Small helper class to pass socket data received from GSIServer on to
user application using a custom event. The event stores a GSISocketData *
using the SetObject() function
*****************************************************************************/
//	ctor
GSISocketData::GSISocketData(int id)
{
	m_id=id;
    m_length=0;
    m_node=GSINotAValidNode;
}
/*****************************************************************************
			ctor
first element of buff holds id, subsequent bytes the record (if any)
*****************************************************************************/
GSISocketData::GSISocketData(GSIChar * buff,wxUint32 len,int node)
{
size_t i;


    SetNode(node);                              //will perform a range test
    wxASSERT(len <= GSIRecordMaxLength+1);      //+1 as we prepend an id to buff before sending
    if(len > GSIRecordMaxLength)
        len=GSIRecordMaxLength;

    m_length=len;
    m_length-=1;                                //ignore the id in the length calculation
    if(m_length <0)
        m_length=0;

    m_id=buff[0];

    wxASSERT(m_id == GSI_SOCKET_RECORD || m_id == GSI_SOCKET_ASYNC_RECORD ||m_id==GSI_SOCKET_LOST);
	if(len)	//beware of the -1 in the for loop!
	{
		for(i=0;i<len-1;i++)
		{
			m_buffer[i]=buff[i+1];
		}
	}

}

	//dtor
GSISocketData::~GSISocketData()
{

}

void GSISocketData::SetNode(int node)
{


    wxASSERT_MSG(node >=0 && node < GSIMaxNumNodes,"node out of range");
    m_node=node;
    if(node <0 || node >= GSIMaxNumNodes)
        m_node=GSINotAValidNode;
}


GSIChar GSISocketData::GetData(size_t index) const
{
	wxASSERT(index <= GSIRecordMaxLength);

	if(index > GSIRecordMaxLength)
		return(0);

	return(m_buffer[index]);
}

errorType GSISocketData::SetData(int index,GSIChar chr)
{
	wxASSERT(index <= GSIRecordMaxLength);
	if(index > GSIRecordMaxLength)
		return(errBounds);
	m_buffer[index]=chr;
	return(errNone);
}



/*************************************************************************************************************
GSISocketReturnData class should be instantiated whenever we have received valid data from a client
If we encounter an error then the class will ensure that error information is sent to the client. The client
will be waiting for this data.
Client can instantiate the class (using a ctor that doesn't take a SocketBase, else it will try to send data!)
to print error data. Can also set the m_internal member to inhibit writing of data
**************************************************************************************************************/


/************************************************************************************
    ctor
SocketReturn::SocketReturn()

*************************************************************************************/
GSISocketReturnData::GSISocketReturnData(wxSocketBase &sock, bool internal_flag)        //set to true if don't want to write to socket
:m_Sock(sock)                                                                           //useful if only want to display data etc.
{

    m_numInts=0;        //default for success
    m_data=NULL;
    m_code=GSI_SERVER_ERROR;
	m_bufferSize=0;
    m_written=false;
    m_internal=internal_flag;
    SetErrorString(GSI_SERVER_ERROR);
    //m_data=m_defaultDataArray;
	m_data=DEBUG_NEW wxInt16[GSIMaxDefaultSocketReturnDataArray];
	if(m_data)
	{
		m_bufferSize=GSIMaxDefaultSocketReturnDataArray*sizeof(wxInt16);
	}
	else
	{
		GSI_THROW("GSISocketReturnData ctor",errMem);
	}
}


/************************************************************************************
    ctor
GSISocketReturnData::GSISocketReturnData()

*************************************************************************************/
GSISocketReturnData::GSISocketReturnData(wxSocketBase &sock,enum GSISocketReturnValue sr,bool flag)
:m_Sock(sock)
{
    m_numInts=0;
    m_code=(wxInt16)sr;
    m_internal=flag;

    m_data=NULL;
    m_numInts=0;        //default for success
    m_written=false;
	m_bufferSize=0;

    SetErrorString(sr);
    //m_data=m_defaultDataArray;
	m_data=DEBUG_NEW wxInt16 [GSIMaxDefaultSocketReturnDataArray];
	if(m_data)
	{
		m_bufferSize=GSIMaxDefaultSocketReturnDataArray*sizeof(wxInt16);
	}
	else
	{
		GSI_THROW("GSISocketReturnData ctor",errMem);
	}
}

/************************************************************************************
    ctor
GSISocketReturnData::GSISocketReturnData()
This ctor for user defined values, outside the range of GSISocketReturnData
*************************************************************************************/
GSISocketReturnData::GSISocketReturnData(wxSocketBase &sock,int sr,bool flag)
:m_Sock(sock)
{
    m_numInts=0;
    m_code=(wxInt16)sr;
    m_internal=flag;

    m_data=NULL;
    m_numInts=0;        //default for success
    m_written=false;
	m_bufferSize=0;

    SetErrorString("User defined");
    //m_data=m_defaultDataArray;
	m_data=DEBUG_NEW wxInt16 [GSIMaxDefaultSocketReturnDataArray];
	if(m_data)
	{
		m_bufferSize=GSIMaxDefaultSocketReturnDataArray*sizeof(wxInt16);
	}
	else
	{
		GSI_THROW("GSISocketReturnData ctor",errMem);
	}
}




/************************************************************************************
    dtor
GSISocketReturnData::~GSISocketReturnData()

*************************************************************************************/
GSISocketReturnData::~GSISocketReturnData()
{

    if(m_written==false)
        Write();            //write the record if not already done so
	delete m_data;
}

/************************************************************************************
GSISocketReturnData::Write
Send back a return code to the client
See enum GSISocketReturnDataValue in startledef.h
*************************************************************************************/
errorType GSISocketReturnData::Write()
{

    if(m_internal == false)     //only write if this is a socket client command
    {
        if(m_numInts && m_data==NULL)
            return errNoAlloc;
        //if(m_numInts >GSIMaxSocketData)
		if(m_numInts > (m_bufferSize/(int) sizeof(wxInt16)) )
            return(errBounds);

        m_Sock.SetFlags(wxSOCKET_WAITALL);
        m_Sock.Write(&m_code, sizeof(m_code));
        m_Sock.Write(&m_numInts,sizeof(m_numInts));

        m_Sock.Write(m_data,m_numInts * sizeof(m_data[0]));
        if(m_Sock.Error())
        {
            return errFail;
        }
        m_written=true;
    }
    SetErrorString((GSISocketReturnValue) m_code);
    return errNone;
}



/*----------------------------------------------------------------
void GSISocketReturnData::SetErrorString(GSISocketReturnValue sr)
-----------------------------------------------------------------*/

void GSISocketReturnData::SetErrorString(GSISocketReturnValue sr)
{

	switch(sr)
    {
    case GSISocketReturnData::GSI_SUCCESS:
        m_ErrorString="Success";
        break;
    case GSISocketReturnData::GSI_FAIL:
        m_ErrorString="Fail";
        break;
    case GSISocketReturnData::GSI_PARAM_ERROR:
        m_ErrorString="Parameter";
        break;
    case GSISocketReturnData::GSI_TIMEOUT:
        m_ErrorString="Timeout";
        break;
    case GSISocketReturnData::GSI_WDT:
        m_ErrorString="Watchdog timer";
        break;
    case GSISocketReturnData::GSI_ALREADY_OPEN:
        m_ErrorString="Already open";
        break;
    case GSISocketReturnData::GSI_NO_SOCKET:
        m_ErrorString="No connected socket";
        break;
    case GSISocketReturnData::GSI_NOT_FOUND:
        m_ErrorString="Not found";
        break;
    case GSISocketReturnData::GSI_OPEN_FAILED:
        m_ErrorString="Open failed";
        break;
    case GSISocketReturnData::GSI_CLOSE_FAILED:
        m_ErrorString="Close failed";
        break;
    case GSI_TOO_MANY_OPEN_DEVICES:
        m_ErrorString="Too many open devices";
        break;
    case GSI_NO_DEVICE:
        m_ErrorString="No device is open";
        break;
    case GSI_SERVER_ERROR:
        m_ErrorString="Server error";
        break;
    default:
        m_ErrorString="Error (no error string defined! Tell Glenn!)";
    }

}

/*----------------------------------------------------------------
void    GSISocketReturnData::SetReturnCode(wxInt16 code)
-----------------------------------------------------------------*/
void    GSISocketReturnData::SetReturnCode(wxInt16 code)
{

    switch(code)
    {
    case GSISocketReturnData::GSI_SUCCESS:
        m_ErrorString="Success";
        break;
    case GSISocketReturnData::GSI_FAIL:
        m_ErrorString="Fail";
        break;
    case GSISocketReturnData::GSI_PARAM_ERROR:
        m_ErrorString="Parameter";
        break;
    case GSISocketReturnData::GSI_TIMEOUT:
        m_ErrorString="Timeout";
        break;
    case GSISocketReturnData::GSI_WDT:
        m_ErrorString="Watchdog timer";
        break;
    case GSISocketReturnData::GSI_ALREADY_OPEN:
        m_ErrorString="Already open";
        break;
    case GSISocketReturnData::GSI_NO_SOCKET:
        m_ErrorString="No connected socket";
        break;
    case GSISocketReturnData::GSI_NOT_FOUND:
        m_ErrorString="Not found";
        break;
    case GSISocketReturnData::GSI_OPEN_FAILED:
        m_ErrorString="Open failed";
        break;
    case GSISocketReturnData::GSI_CLOSE_FAILED:
        m_ErrorString="Close failed";
        break;
    case GSI_TOO_MANY_OPEN_DEVICES:
        m_ErrorString="Too many open devices";
        break;
    case GSI_NO_DEVICE:
        m_ErrorString="No device is open";
        break;
    case GSI_SERVER_ERROR:
        m_ErrorString="Server error";
        break;
    default:
        m_ErrorString="Error (no error string defined! Tell Glenn!)";
	}
    //m_code=(GSISocketReturnValue)code;
    m_code=code;

}

/*!
GSISocketReturnValue GSISocketReturnData::SetReturnCode(errorType rv)
convert errorType to GSI GSISocketReturnValue
*/
GSISocketReturnData::GSISocketReturnValue GSISocketReturnData::SetReturnCode(errorType rv)
{
	SetReturnCode(MapError(rv));
	return( static_cast <GSISocketReturnData::GSISocketReturnValue> (GetReturnCode() ) );
}

/*!
	GSISocketReturnValue GSISocketReturnData::MapError(errorType error)
convert from errorType to GSIReturnValue
   GSI_SUCCESS=0,
    GSI_FAIL=1,
    GSI_PARAM_ERROR=2,
    GSI_TIMEOUT=3,
    GSI_WDT=4,
    GSI_ALREADY_OPEN=5,
    //GSINoNode=6,
	GSI_NO_SOCKET=6,
    GSI_NOT_FOUND=7,
    GSI_OPEN_FAILED=8,
    GSI_CLOSE_FAILED=9,
    GSI_TOO_MANY_OPEN_DEVICES=10,
	GSI_NO_DEVICE=11,

    GSI_SERVER_ERROR=50,

    GSI_NOT_A_RETURN_VALUE=98,			//derived classes can use id's >
	GSI_FIRST_USER_GSI_SOCKET_VALUE=99	//User code can do something
*/
GSISocketReturnData::GSISocketReturnValue GSISocketReturnData::MapError(errorType error)
{
GSISocketReturnValue rc;

	switch(error)
	{
	case errParameter: //
		rc=GSI_PARAM_ERROR;
		break;
	case errTimeout: //
		rc=GSI_TIMEOUT;
		break;
	case errAlreadyOpen:  //  Invalid address passed to wxSocket.
	case errIsOpen:
		rc=GSI_ALREADY_OPEN;
		break;
	case errSocket:
	case errSocketLost:
	case errSocketInvalid:
	case errSocketNoHost:
		rc=GSI_NO_SOCKET;
		break;
	case errFileOpen:
		rc=GSI_OPEN_FAILED;
		break;
	case errFileClose:
		rc=GSI_CLOSE_FAILED;
		break;
	case errNotOpen:
	case errNotExist:
	case errBadDevice:
		rc=GSI_NO_DEVICE;
		break;
	case errUnknown:
		rc=GSI_NOT_A_RETURN_VALUE;
		break;
	case errFail:
	case errSocketRead:
	case errSocketWrite:
	default:
		rc=GSI_FAIL;
		break;
	}
	return rc;
}

/*!
GSISocketReturnData::ResizeDataBuff(int num_elements)
delete the existing default sized buffer and allocate
a new one.
Allows us to send back more data through the socket
*/
errorType GSISocketReturnData::ResizeDataBuff(int num_elements)
{
	if(m_data !=NULL)
	{
		delete m_data;
		m_data=DEBUG_NEW wxInt16 [num_elements];
		if(m_data)
		{
			m_bufferSize=num_elements*sizeof(wxInt16);
			return errNone;
		}
		else
		return errMem;
	}
	return errFail;
}




/****************************************************************************************************************************************
    ctor
GSISocketException::GSISocketException(errorType e, const wxString &s="",unsigned int data0=0,unsigned int data1=0, unsigned int data2=0)

*****************************************************************************************************************************************/
GSISocketException::GSISocketException(const wxSocketBase &sock,errorType e, const wxString &s,unsigned int data0,unsigned int data1, unsigned int data2)
:gsException(e,s,data0,data1,data2),m_Sock(sock)
{
}

/****************************************************************************************************************************************
    dtor
GSISocketException::~GSISocketException()
*****************************************************************************************************************************************/

GSISocketException::~GSISocketException()
{
}

/************************************************************
	MapWxSocketError::MapError(enum wxSocketError error)

Convert from wxSocketError to errorType
************************************************************/


MapWxSocketError::MapWxSocketError()
{
	m_rv=errError;				//error in error handling.
}


MapWxSocketError::MapWxSocketError(enum wxSocketError error)
{
	MapError(error);			//sets m_rv
}


MapWxSocketError::~MapWxSocketError()
{
}


errorType MapWxSocketError::MapError(enum wxSocketError error)
{
	switch(error)
	{
	case wxSOCKET_NOERROR:	//  No error happened.
		m_rv=errNone;
		break;
	case wxSOCKET_INVOP: //  Invalid operation.
		m_rv=errInVal;
		break;
	case wxSOCKET_IOERR: //  Input/Output error.
		m_rv=errIO;
		break;
	case wxSOCKET_INVADDR:  //  Invalid address passed to wxSocket.
		m_rv=errBadAddress;
		break;
	case wxSOCKET_INVSOCK: //  Invalid socket (uninitialized).
		m_rv=errSocketInvalid;
		break;
	case wxSOCKET_NOHOST: //  No corresponding host.
		m_rv=errSocketNoHost;
		break;
	case wxSOCKET_INVPORT: //  Invalid port.
		m_rv=errBadPort;
		break;
	case wxSOCKET_WOULDBLOCK: //  The socket is non-blocking and the operation would block.
		m_rv=errWouldBlock;
		break;
	case wxSOCKET_TIMEDOUT: //  The timeout for this operation expired.
		m_rv=errTimeout;
		break;
	case wxSOCKET_MEMERR: //  Memory exhausted.
		m_rv=errOutOfMem;
		break;
	default:
		m_rv=errUnknown;
		break;
	}
	return m_rv;
}








#if 0
 Interesting:
 This fails to compile with link errors or body already defined errors if implemented here
 However if implemented in the header file as inline code, it works fine!
 /****************************************************************************************
 wxGSISocketCommandEvent::~wxGSISocketCommandEvent()     //delete any add GSIObject here
******************************************************************************************/
wxGSISocketCommandEvent::~wxGSISocketCommandEvent()     //delete any add GSIObject here
{
//might not need to do anything. When we SetClientObject the old object will be deleted, thus seems logical to think
//that any object here will also be deleted when the event is deleted!
#warning ("Check out this")   //xxx here
int i=1;
    i++;
}


wxEvent * wxGSISocketCommandEvent::Clone() const
{
    return DEBUG_NEW wxGSISocketCommandEvent(*this);
}   //should this clone the ClientObject as well??





#endif
