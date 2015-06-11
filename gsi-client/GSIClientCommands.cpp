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
#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "GSIClientCommands.cpp"
#  pragma interface "GSIClientCommands.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP

#  include "wx/wx.h"
#endif
#include <wx/wx.h>
#include "wx/socket.h"
#include <wx/stopwatch.h>
#include <wx/progdlg.h>


//#include "gsi-common/include/gsi.hpp"
#include "gsi-common/include/gsbexcept.hpp"
#include "gsi-common/include/record.hpp"
//#include "include/server.hpp"
#include "gsi-common/include/socket.hpp"
#include "include/GSIClientCommands.hpp"

//#ifdef STARTLE_SERVER
//#include "startle.hpp"
//#endif


#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#else
    #define DEBUG_NEW new
#endif



//GSIClientCommandXXX start

/**************************************************************************************************
    ctor
GSIClientCommand::GSIClientCommand(wxSocketBase & sock,wxTextCtrl * const status_text)
**************************************************************************************************/
GSIClientCommandXXX::GSIClientCommandXXX(wxSocketBase & sock)
:m_Sock(sock)
//:GSICommandXXX(status_text),
{
#if __VISUALC__
#warning ("socket client timeout set to 1000 seconds")
#endif
	sock.SetTimeout(1000);
	Init();
}
/**************************************************************************************************
    dtor
GSIClientCommandXXX::~GSIClientCommandXXX()
**************************************************************************************************/
GSIClientCommandXXX::~GSIClientCommandXXX()
{
    if(IsRead()==false)
    {
        wxFAIL_MSG("Haven't read ServerReturnRecord (or failed to set flag in Read())");

    }

	if(GetReturnData())
		delete GetReturnData();

}

/*******************************************************
void GSIClientCommandXXX::Init()
*******************************************************/
void GSIClientCommandXXX::Init()
{
	SetWrittenFlag(false);
	SetReadFlag(false);
	m_serverReturnData=NULL;
	m_serverReturnCode=GSISocketReturnData::GSI_FAIL;
    m_serverReturnNumInts=0;

}

int GSIClientCommandXXX::GetReturnCode() const
{
	return ((int) m_serverReturnCode);
}
/**************************************************************************************************
    errorType GSIClientCommand::ReadServerReturnRecord()
**************************************************************************************************/
errorType GSIClientCommandXXX::ReadServerReturnRecord()
{
errorType rv;
    GetSock().SetFlags(wxSOCKET_WAITALL);

    GetSock().Read(&m_serverReturnCode,sizeof(m_serverReturnCode));

    if(GetSock().Error())
    {
        rv=errFail;
        GSI_SOCKET_THROW(GetSock(),"ReadServerReturnRecord",errSocket);
        wxFAIL_MSG("ReadServerReturnRecord");
    }
    GetSock().Read(&m_serverReturnNumInts,sizeof(m_serverReturnNumInts));
    if(GetSock().Error())
    {
        rv=errFail;
        GSI_SOCKET_THROW(GetSock(),"ReadServerReturnRecord",errSocket);
        wxFAIL_MSG("ReadServerReturnRecord");
    }

    //wxASSERT_MSG(GetReturnNumInts() < GSIMaxSocketData, "Too much data returned by socket server");
    if(m_serverReturnData)
    {
        delete m_serverReturnData;      //delete if previously allocated
        m_serverReturnData=NULL;
    }

    m_serverReturnData=DEBUG_NEW wxInt16[m_serverReturnNumInts];
    GetSock().Read(&m_serverReturnData[0],m_serverReturnNumInts*sizeof(wxInt16));
    if(GetSock().Error())
    {
        rv=errFail;
        GSI_SOCKET_THROW(GetSock(),"ReadServerReturnRecord",errSocket);
        wxFAIL_MSG("ReadServerReturnRecord");
    }
    SetReadFlag(true);
    return errNone;
}


/**************************************************************************************************
    errorType GSIClientCommandXXX::Write()
**************************************************************************************************/
errorType GSIClientCommandXXX::Write(bool flag)      //all derived must know how to write their own data!
{
	flag;
	wxFAIL_MSG("Not Implemented GSICLientCommand::Write(bool flag)");
	return errNotImplemented;
}

errorType GSIClientCommandXXX::Write(GSICommandXXX & STLCmd,bool flag)      //all derived must know how to write their own data!
{
	STLCmd;
	flag;
	wxFAIL_MSG("Not Implemented GSICLientCommandXXX::Write(GSICommand &STLCmd,bool flag)");
	return errNotImplemented;
}



//-------------------------------------------------------------------------------------------------------------


/**************************************************************************************************
            class GSIClientCommandOpenDeviceXXX
**************************************************************************************************/


/**************************************************************************************
	GSIClientCommandOpenDeviceXXX::GSIClientCommandOpenDeviceXXX(wxSocketBase &sock,GSICommand &GSICmd, const wxString & device_str,wxTextCtrl * const status_text)
	ctor
***************************************************************************************/


GSIClientCommandOpenDeviceXXX::GSIClientCommandOpenDeviceXXX(wxSocketBase &sock,const wxString & device_str,wxTextCtrl * const status_text)
:GSIClientCommandXXX(sock),GSICommandXXX(status_text)//:m_Sock(sock), m_StatusText(text)

{
errorType rv=errNone;
    m_DeviceStr=device_str;

    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandOpenDeviceXXX",errSocket);
        return;
    }
    int n=GetReturnNumInts();
    if(n==1)            //will return handle to this device
    {
        wxInt16 *data;
        data=GetReturnData();
        m_hDevice=*data ;
    }
    else
    {   //can return less data on a failure

    }

    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}



GSIClientCommandOpenDeviceXXX::~GSIClientCommandOpenDeviceXXX()
{
}



/********************************************************************
    errorType GSIClientCommandOpenDeviceXXX::Write(bool use_written_flag)
Used by a GSISocketClient to write to a GSISocketServer

Write:
WxInt16 Length of string
char[length of string]  (The device name)
********************************************************************/
errorType GSIClientCommandOpenDeviceXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");


    GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending OpenDevice record-\n"));
//-----------Write the socket command type id (A socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_RECORD;
    //GetSock().Write(&buffer,sizeof (wxInt16));                    //write the id
    GetSock().Write(&buffer,sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Write the GSICommand id
    buffer=GSI_COMMAND_OPEN_DEVICE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Above are read by the Socket event handler

//below are read by the specific read function associated with this object

//----write the data length in bytes-----------------       //socket server will understand this format
    buffer=(wxInt16)m_DeviceStr.length();                   //length of data in bytes
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
        if(GetSock().Error())
            return(errFail);
//-----write the data sring in this instance ----
    GetSock().Write(m_DeviceStr.c_str(),m_DeviceStr.length());   //write the str
    if(GetSock().Error())
        return(errFail);
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandOpenDevice",errSocket);
    }

    return errNone;
}



/**************************************************************************************************
            class GSIClientCommandCloseDeviceXXX
**************************************************************************************************/


/**************************************************************************************
    errorType GSIClientCommandXXX::CloseDevice(const wxString & device)
This class can be instantiated by a sock server or client. Calling write from a server
or Read from a client will be fatal, unless the class is modified to accept the data transfer.
Currently it only works, and only has meaning, one way.
***************************************************************************************/

GSIClientCommandCloseDeviceXXX::GSIClientCommandCloseDeviceXXX(wxSocketBase & sock,GSIHandle handle,wxTextCtrl * const status_text)
:GSIClientCommandXXX(sock),GSICommandXXX(status_text)
{
errorType rv=errNone;
    SetReadFlag(true);                   //client doesn't need to read the record, so pretend we have read it
                                        //otherwise the dtor will attempt to read!
    m_deviceHandle=handle;
    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandCloseDevice",errSocket);
        return;
    }
//-----record written-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandCloseDevice",errSocket);
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}



GSIClientCommandCloseDeviceXXX::~GSIClientCommandCloseDeviceXXX()
{
}

/********************************************************************
    errorType GSIClientCommandCloseDeviceXXX::Write(bool use_written_flag)

Write:
GSIHandle device handle
********************************************************************/
errorType GSIClientCommandCloseDeviceXXX::Write(bool use_written_flag)
{
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");


    GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending CloseDevice record-\n"));
//-----------Write the socket command type id (A socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_RECORD;
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Write the GSICommand id
    buffer=GSI_COMMAND_CLOSE_DEVICE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);


//Above are read by the Socket event handler

//below are read by the specific read function associated with this object

//-----write the data a GSIHandle in this instance ----
    GetSock().Write(&m_deviceHandle,sizeof(m_deviceHandle));   //write the GSIHandle
    if(GetSock().Error())
        return(errFail);
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        SetWrittenFlag(true);
    }
    return errNone;
}



/*!
* errorType GSIClientCommandCloseDeviceXXX::SetDeviceHandle(GSIHandle h)
*/

errorType GSIClientCommandCloseDeviceXXX::SetDeviceHandle(GSIHandle h)
{
errorType rv=errNone;
    m_deviceHandle=GSINotAValidDevHandle;

    if(h>0 && h<GSIMaxNumDeviceHandles)
    {
        m_deviceHandle=h;
        rv=errRange;
    }

    return rv;
}



//------------GSIClientCommandRemoveNodeXXX(int node)-------------------


/**************************************************************************************
GSIClientCommandRemoveNodeXXX::GSIClientCommandRemoveNodeXXX(wxSocketBase &sock, GSINodeHandle h_node, GSIDevHandle h_device,wxTextCtrl *text, bool send_now)
This ctor is for Client access
This class can be instantiated by a sock server or client. Calling write from a server
or Read from a client will be fatal, unless the class is modified to accept the data transfer.
Currently it only works, and only has meaning, one way.
***************************************************************************************/

GSIClientCommandRemoveNodeXXX::GSIClientCommandRemoveNodeXXX(wxSocketBase &sock,GSINodeHandle h_node, GSIDevHandle h_device,wxTextCtrl * const status_text) //client side
:GSIClientCommandXXX(sock),GSICommandXXX(status_text)//:m_Sock(sock), m_StatusText(text)
{
errorType rv=errNone;
	h_device;

    SetNodeHandle((wxInt16) h_node);
    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandRemoveNode",errSocket);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}



/***************************************************************
GSICommandRemoveNodeXXX::~GSICommandRemoveNodeXXX()
***************************************************************/
GSIClientCommandRemoveNodeXXX::~GSIClientCommandRemoveNodeXXX()
{
}

/********************************************************************
    errorType GSIClientCommandRemoveNodeXXX::Write(bool use_written_flag)
Used by a GSISocketClient to write to a GSISocketServer

write:
GSIHandle node to remove
********************************************************************/
errorType GSIClientCommandRemoveNodeXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");


    GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending RemoveNode record-\n"));
//-----------Write the socket command type id (A socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_RECORD;
    GetSock().Write(&buffer, sizeof(wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Write the GSICommand id
    buffer=GSI_COMMAND_REMOVE_NODE;                           //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Above are read by the Socket event handler

//below are read by the specific read function associated with this object
//-----write the data wxInt16 in this instance ----
    GetSock().Write(&m_nodeHandle,sizeof(m_nodeHandle));                        //write the node we wish to remove
    if(GetSock().Error())
        return(errFail);
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandOpenDevice",errSocket);
    }

    return errNone;
}


const GSIHandle  GSIClientCommandRemoveNodeXXX::GetNode() const
{

    return m_nodeHandle;
}


/**************************************************************
void GSIClientCommandRemoveNodeXXX::SetNodeHandle(GSINodeHandle h)
***************************************************************/
void GSIClientCommandRemoveNodeXXX::SetNodeHandle(GSINodeHandle h)
{

	if(h==GSINotAValidNodeHandle)
		return;
	if(h>GSIMaxNumNodeHandles || h <0)
	{
		m_nodeHandle=GSINotAValidNodeHandle;
		GSI_THROW("GSIClientCommandRemoveNode::SetNodeHandle",errBadHandle)
	}
	m_nodeHandle=h;
}


//------------GSIClientCommandAddNodeXXX(int node)-------------------


/**************************************************************************************

    ctor (client side)
    GSIClientCommandAddNodeXXX(int node)

Tell server that we wish to receive messages from mc node 'node' on device device_handle
handle obtained through a previous call to OpenDevice
If the node already has a socket with this address it will return an error
indeed, currently only supports one socket for each node.
If node doesn't exist, then will still create a node but it will not function. If it comes
online at a later date then it should begin to function
***************************************************************************************/
//GSIClientCommandAddNodeXXX::GSIClientCommandAddNodeXXX(wxSocketBase &sock, GSIHandle h_node, GSIHandle h_device,wxTextCtrl *text)
GSIClientCommandAddNodeXXX::GSIClientCommandAddNodeXXX(wxSocketBase &sock,GSINodeHandle h_node, GSIDevHandle h_device,wxTextCtrl * const status_text) //client side
:GSIClientCommandXXX(sock),GSICommandXXX(status_text)
{
errorType rv=errNone;

	wxASSERT_MSG(h_node >=0 && h_node <= GSIMaxNumNodeHandles,"Bad node handle");

    SetNodeHandle(h_node);
    SetDeviceHandle(h_device);

    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandAddNode",errSocket);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}



/***************************************************************
GSIClientCommandAddNodeXXX::~GSIClientCommandAddNodeXXX()
***************************************************************/
GSIClientCommandAddNodeXXX::~GSIClientCommandAddNodeXXX()
{


}



/********************************************************************
    errorType GSIClientCommandAddNodeXXX::Write(bool use_written_flag)
Used by a GSISocketClient to write to a GSISocketServer
********************************************************************/
errorType GSIClientCommandAddNodeXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");


    GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending AddNode record-\n"));
//-----------Write the socket command type id (A socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_RECORD;
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Write the GSICommand id
    buffer=GSI_COMMAND_ADD_NODE;                           //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Above will be read by the general socket read function

//following will be read by the corresponding Read() function belonging to this object

//------Write device handle---------
    GetSock().Write(&m_deviceHandle, sizeof (m_deviceHandle));                    //write the data length a wxInt16
        if(GetSock().Error())
            return(errFail);

//-----write the node handle ----
    GetSock().Write(&m_nodeHandle,sizeof(m_nodeHandle));                        //write the node we wish to add, if no network will open GSINodeThis
    if(GetSock().Error())
        return(errFail);
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandAddNode",errSocket);
    }

    return errNone;
}


const GSINodeHandle  GSIClientCommandAddNodeXXX::GetNodeHandle() const
{
    return m_nodeHandle;
}




/*!
* errorType GSIClientCommandAddNodeXXX::SetNodeHandle(GSIHandle h)
*/

/*************************************************************************
errorType GSIClientCommandAddNodeXXX::SetNodeHandle(GSIHandle h)

*************************************************************************/

errorType GSIClientCommandAddNodeXXX::SetNodeHandle(GSIHandle h)
{
errorType rv=errNone;

    if( h<0 || h >= GSIMaxNumNodeHandles)
    {
        h=GSINotAValidNodeHandle;
        rv=errRange;
    }
    m_nodeHandle=h;

    return rv;
}

/*!
* errorType GSIClientCommandAddNodeXXX::SetDeviceHandle(GSIHandle h)
*/

errorType GSIClientCommandAddNodeXXX::SetDeviceHandle(GSIHandle h)
{
errorType rv=errNone;

	m_deviceHandle=GSINotAValidDevHandle;

    if(h<0 || h >= GSIMaxNumDeviceHandles)
    {
		m_deviceHandle=GSINotAValidDevHandle;
        rv=errRange;
    }
    else
       m_deviceHandle=h;

    return rv;
}


/*!
*void GSIClientCommandXXX::DisplayReturnCode()
*/

void GSIClientCommandXXX::DisplayReturnCode(wxTextCtrl *status_text)
{
	if(status_text)
	{
		wxString s;
		GSISocketReturnData rd(GetSock(),true);        //set internal flag so we don't write to the socket

		rd.SetReturnCode(m_serverReturnCode);

		s.Printf("\nServer returned code %i (%s)\n",rd.GetReturnCode(),rd.GetErrorString().c_str());
		status_text->AppendText(s);
	}
}


/*--------------------------------------------------------
void GSIClientCommandXXX::DisplayData()
---------------------------------------------------------*/

void GSIClientCommandXXX::DisplayData(wxTextCtrl * status_text)
{
	if(status_text)
	{
	wxString s,s1;
	int i;

		if(m_serverReturnNumInts)
		{

			s.Printf("\nServer returned %i data items\nData:",m_serverReturnNumInts);
			for (i=0; i<m_serverReturnNumInts && i <10;i++)
			{
				s1.Printf("%i ",m_serverReturnData[i]);
				s=s+s1;
			}
			s1.Printf("\n");
			s=s+s1;
			status_text->AppendText(s);
		}
		status_text->AppendText(_("----------------------------------------------\n"));
	}
}

void GSIClientCommandXXX::SetReturnCode(wxInt16 code)
{
	m_serverReturnCode=code;
//instantiate a GSISocketReturnData, with flag so we don't write

	GSISocketReturnData rd(GetSock(),true);        //set internal flag so we don't write to the socket

    rd.SetReturnCode(m_serverReturnCode);
	SetClientErrorString(rd.GetErrorString());
}

