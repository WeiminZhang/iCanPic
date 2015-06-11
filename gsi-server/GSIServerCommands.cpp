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
#  pragma implementation "GSIServerCommands.cpp"
#  pragma interface "GSIServerCommands.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#include "wx/socket.h"
#include <wx/stopwatch.h>
#include <wx/progdlg.h>


#include "include/gsi.hpp"
#include "include/gsbexcept.hpp"
#include "include/record.hpp"
#include "include/server.hpp"
#include "include/socket.hpp"
#include "include/GSIServerCommands.hpp"
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


/*!
      class GSIServerCommandCloseDevice
			ctor
*/

GSIServerCommandCloseDeviceXXX::GSIServerCommandCloseDeviceXXX(wxSocketBase & sock)
:GSICommandCloseDeviceXXX(),GSISocketXXX(sock)
{
	SetCommandString("GSIServerCommandCloseDeviceXXX");
}

/*!
	GSIServerCommandCloseDeviceXXX::~GSIServerCommandCloseDeviceXXX()
		dtor
*/
GSIServerCommandCloseDeviceXXX::~GSIServerCommandCloseDeviceXXX()
{
	AppendText("Called GSIServerCommandCloseDeviceXXX\n");
}



/*!
    errorType GSIServerCommandCloseDevice::Read(const wxString &m_device)

  will have already read the Socket command type and the command id

Should only be called from a GSISocket server
polymorphic Read()
*/
errorType GSIServerCommandCloseDeviceXXX::Read(wxSocketBase &sock)
{
//wxInt16 length;

    GetSock().SetFlags(wxSOCKET_WAITALL);

//-----Read the data, a GSIHandle in this instance ----
    GSIHandle h;
    GetSock().Read(&h,sizeof(h));   //Read the handle
    if(GetSock().Error())
        return(errFail);
    //m_deviceHandle=h;
    SetDeviceHandle(h);			//use the access functions as they perform error checking
    SetReadFlag(true);
    return errNone;
}


/*!
* errorType GSIServerCommandCloseDeviceXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
*/
errorType GSIServerCommandCloseDeviceXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errFail;
	sr;
	rv=Frame.CloseDevice(GetDeviceHandle());
	return rv;
}




//-------------End GSIServerCommandCloseDeviceXXX--------------



/*!
    errorType GSIServerCommandOpenDeviceXXX::GSIServerCommandOpenDeviceXXX(const wxString & device)
		ctor
Read the device name from the socket client
# wxInt16 lengthOfSstring
# char buff[lengthOfString]

*/
GSIServerCommandOpenDeviceXXX::GSIServerCommandOpenDeviceXXX(wxSocketBase &sock)
:GSICommandOpenDeviceXXX(), GSISocketXXX(sock)
{
//GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure
//
	SetCommandString("GSIServerCommandOpenDeviceXXX");
}


/*!
GSIServerCommandOpenDeviceXXX::~GSIServerCommandOpenDeviceXXX()
*/
GSIServerCommandOpenDeviceXXX::~GSIServerCommandOpenDeviceXXX()
{
}

/**********************************************************************************************
    errorType GSIServerCommandOpenDevice::Read(const wxString &m_device)
;will have already read the Socket command type and the command id
A polymorphic Read()
***********************************************************************************************/
//errorType GSIServerCommandOpenDeviceXXX::Read()
errorType	GSIServerCommandOpenDeviceXXX::Read(wxSocketBase &sock) //read from Socket
{
wxInt16 length;

    GetSock().SetFlags(wxSOCKET_WAITALL);

//Sending a string, so first send its length, followed by the buffer
//----read the data length in bytes-----------------       //socket server will understand this format
    GetSock().Read(&length,sizeof(wxInt16));
        if(GetSock().Error())
            return(errFail);

//-----Read the data, a string in this instance ----
    char *buff=DEBUG_NEW char[length];
    GetSock().Read(buff,length);   //Read the str
    SetDeviceStr(wxString(buff, length));
    delete buff;
    if(GetSock().Error())
        return(errFail);
    SetReadFlag(true);
    return errNone;
}

/*!
errorType GSIServerCommandOpenDeviceXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
*/
errorType GSIServerCommandOpenDeviceXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)	//execute the class specific command
{
GSIDevHandle hDev;
errorType rv;

	rv=Frame.OpenDevice(GetDeviceString(),&hDev);
	SetDeviceHandle(hDev);
	wxInt16 * const array=sr.GetDataArray();
	array[0]=GetDeviceHandle();                                        //return the handle
	sr.SetNumInts(1);
	return rv;
}

//------End GSIServerCommandOpenDeviceXXX----



/**************************************************************************************************
    ctor
GSIServerCommandXXX::GSIServerCommandXXX(wxSocketBase & sock, wxTextCtrl &text,bool send_now)
**************************************************************************************************/
GSIServerCommandXXX::GSIServerCommandXXX(wxSocketBase & sock)
:m_Sock(sock)
{
	Init();
}

/**************************************************************************************************
    dtor
GSIServerCommandXXX::~GSIServerCommandXXX()
**************************************************************************************************/
GSIServerCommandXXX::~GSIServerCommandXXX()
{

    //if(GetReturnData())
    //    delete GetReturnData();
}


/*!
void GSIServerCommandXXX::Init()
*/
void GSIServerCommandXXX::Init()
{
	SetReadFlag(false);
}


/*!
void GSIServerCommandXXX::Read()
OnSocketEvent() read the socket event type
GSI_SOCKET_RECORD,              //these must be between 0 and 255
GSI_SOCKET_ASYNC_RECORD,
...
GSI_SOCKET_USER1, etc

Then calls  GSIServer::ProcessRecordFromClient(sock); //GSI_SOCKET_RECORD
or			GSISERver::ProcessUser1Record(sock);	//GSISOCKET_USER1

GSIServer::ProcessRecordFromClient currently reads the id then calls the
correct GSIFrame::Func(sock) command

What it _probably_ should do, is instantiate the appropriate polymorphic GSIServerCommand
The GSIServerCommand() will then read the rest of the socket data

Same is true of STLServerCommands that need to be called from:
GSIServer::ProcessRecordFromClient currently reads the id then calls the
polymorphic StartleServer::ProcessUser1Record(wxSocketBase &sock)
StartleServer::Processxxx should instantiate appropriate polymorphic
STLServerCommand() which will know how to read its data

*/
errorType GSIServerCommandXXX::Read()
{
	wxFAIL_MSG("Implement simple socket read for GSIRecord type,node,length??");
//Might be best for server to read the fixed length record stuff
	SetReadFlag(true);
	return errNotImplemented;
}
// ---------End GSIServerCommandXXX --------------------


//------------GSIServerCommandRemoveNodeXXX(int node)-------------------

/**************************************************************************************
GSIServerCommandRemoveNodeXXX::GSIServerCommandRemoveNodexxx(wxSocketBase &sock, GSINodeHandle h_node)
***************************************************************************************/

GSIServerCommandRemoveNodeXXX::GSIServerCommandRemoveNodeXXX(wxSocketBase &sock)
:GSIServerCommandXXX(sock)
{

}

/***************************************************************
GSICommandRemoveNodeXXX::~GSICommandRemoveNodeXXX()
***************************************************************/
GSIServerCommandRemoveNodeXXX::~GSIServerCommandRemoveNodeXXX()
{
}


/*************************************************************************
    errorType GSIServerCommandRemoveNodeXXX::Read()
;will have already read the Socket command type and the command id

Should only be called from a GSISocket server
*************************************************************************/
errorType GSIServerCommandRemoveNodeXXX::Read(wxSocketBase &sock)
{

//-----Read the data, a simple wxInt16 in this instance ----

    GetSock().Read(&m_nodeHandle,sizeof(m_nodeHandle));   //Read the data
    if(GetSock().Error())
        return(errFail);
    SetReadFlag();
    return errNone;
}

const GSIHandle  GSIServerCommandRemoveNodeXXX::GetNodeHandle() const
{

    return m_nodeHandle;
}

/*!
errorType GSIServerCommandRemoveNodeXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
*/
errorType GSIServerCommandRemoveNodeXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
{
	sr;
	return(Frame.RemoveNode(GetNodeHandle()));
}

//------------End GSIServerCommandRemoveNodeXXX------------


/*********************************************************************************************
          ctor (server side)
GSIServerCommandAddNodeXXX::GSIServerCommandAddNodeXXX(wxSocketBase &sock)
**********************************************************************************************/
GSIServerCommandAddNodeXXX::GSIServerCommandAddNodeXXX(wxSocketBase &sock)
:GSIServerCommandXXX(sock)
{
	SetCommandString("GSICommandAddNodeXXX");
}

/***************************************************************
GSIServerCommandAddNodeXXX::~GSIServerCommandAddNodeXXX()
***************************************************************/
GSIServerCommandAddNodeXXX::~GSIServerCommandAddNodeXXX()
{

    if(IsRead()==false)
    {
        //Read(); c
        //XXXGS Changed above to use Read(sock) 12/3/2012 completely untested
        Read(GetSock());         //although this will throw away the handle from the client, it will not be waiting on socket data!
        wxFAIL_MSG("Failed to read the socket data for CommandAddNodeXXX()");
    }

}



/*************************************************************************
    errorType GSIServerCommandAddNodeXXX::Read()
;will have already read the Socket command type and the command id

read two items:
GSIHandle m_hDevice :handle of device to look for node
GSIHandle m_hNode    : requested node to open. Will return GSINodeThis if no network
*************************************************************************/
errorType GSIServerCommandAddNodeXXX::Read(wxSocketBase &sock)
{
	GSINodeHandle hnode;

    GetSock().SetFlags(wxSOCKET_WAITALL);
//----read the device handle-----------------       //socket server will understand this format
    GetSock().Read(&m_deviceHandle,sizeof(m_deviceHandle));
        if(GetSock().Error())
            return(errFail);

//-----Read the node handle ----
    GetSock().Read(&hnode,sizeof(hnode));   //Read the data
    if(GetSock().Error())
        return(errFail);
    SetReadFlag();
	SetNodeHandle(hnode);		//will perform error checking
    return errNone;
}

const GSINodeHandle  GSIServerCommandAddNodeXXX::GetNodeHandle() const
{
    return m_nodeHandle;
}




/*!
* errorType GSIServerCommandAddNodeXXX::SetNodeHandle(GSIHandle h)
*/

/*************************************************************************
errorType GSIServerCommandAddNodeXXX::SetNodeHandle(GSIHandle h)

Should only be called from a GSISocket server, only has meaning there
*************************************************************************/

errorType GSIServerCommandAddNodeXXX::SetNodeHandle(GSIHandle h)
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
* errorType GSIServerCommandXXXAddNode::SetDeviceHandle(GSIHandle h)
*/

errorType GSIServerCommandAddNodeXXX::SetDeviceHandle(GSIHandle h)
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
* errorType GSIServerCommandAddNodeeXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
*/
errorType GSIServerCommandAddNodeXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errFail;
	rv=Frame.CreateNode(GetDeviceHandle(),GetNodeHandle());
	if(rv==errNone)
	{
		wxInt16 * const array=sr.GetDataArray();
		array[0]=GetNodeHandle();                                        //return the handle
		sr.SetNumInts(1);
	}

	return rv;
}
