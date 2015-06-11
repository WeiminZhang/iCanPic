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

//#if defined(__GNUG__) && !defined(__APPLE__)
//    #pragma implementation
//    #pragma interface
//#endif
#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "server.cpp"
#  pragma interface "server.cpp"
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
#include "include/gsi.hpp"
#include "include/socket.hpp"
#include "include/server.hpp"
#include "include/GSIServerCommands.hpp"

//#if STARTLE_SERVER
//#include "startle.hpp"
//#endif


//#include "lcdClient.hpp"

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

BEGIN_EVENT_TABLE(GSIServer, wxEvtHandler)
  EVT_SOCKET(GSI_SERVER_ID,  GSIServer::OnServerEvent)
  EVT_SOCKET(GSI_SOCKET_ID,  GSIServer::OnSocketEvent)

END_EVENT_TABLE()

GSIServer::GSIServer(unsigned short sock_address,wxStatusBar *status_bar,wxTextCtrl *text_ctrl, GSIFrame *gsi_frame)// :m_Node(node)
{
	m_dummyTextCtrlFlag=false;
    m_TextCtrl=text_ctrl;
	if(m_TextCtrl==NULL) //no text output
	{
		m_dummyTextCtrlFlag=true;
		m_TextCtrl= DEBUG_NEW wxTextCtrl;
	}
    m_StatusBar=status_bar;
    m_GSIFrame=gsi_frame;
    // Create the address - defaults to localhost:0 initially
    wxIPV4address addr;
    addr.Service(sock_address);

    // Create the socket
    m_Server = DEBUG_NEW wxSocketServer(addr);

    // We use Ok() here to see if the server is really listening
    if (! m_Server->Ok())
    {
    TextCtrl()->AppendText(_("Could not listen at the specified port !\n\n"));
    return;
    }
    else
    {
    TextCtrl()->AppendText(_("Server listening.\n\n"));
    }

    // Setup the event handler and subscribe to connection events
    m_Server->SetEventHandler(*this, GSI_SERVER_ID);
    m_Server->SetNotify(wxSOCKET_CONNECTION_FLAG);
    m_Server->Notify(true);

    m_busy = false;
    m_numClients = 0;
    m_GSIClient=NULL;																		//the address of the GSIClient socket

}

GSIServer::~GSIServer()
{
	if(m_dummyTextCtrlFlag==true)
		delete m_TextCtrl;
    delete m_Server;
}


/************************************************************
void GSIServer::OnServerEvent(wxSocketEvent& event)
************************************************************/
void GSIServer::OnServerEvent(wxSocketEvent& event)
{
  //wxString s = _("OnServerEvent: ");
  //wxSocketBase *sock;

    switch(event.GetSocketEvent())
    {
    case wxSOCKET_CONNECTION:
        break;
    default:
	    break;
    }

  //m_text->AppendText(s);

  // Accept new connection if there is one in the pending
  // connections queue, else exit. We use Accept(false) for
  // non-blocking accept (although if we got here, there
  // should ALWAYS be a pending connection).

    if(m_GSIClient)	//Do we already have a connection?
    {
	    return;			//refuse connection
	}

	m_GSIClient=m_Server->Accept(false);

    if (NumClients()==0)
    {
	    TextCtrl()->AppendText(_("New client connection accepted\n\n"));
        if (m_GSIFrame)
            m_GSIFrame->SetIcon(wxICON(CONN_ICN));
	}
	else
	{
	    TextCtrl()->AppendText(_("Error: couldn't accept a new connection\n\n"));

		return;
	}

		// Tell the new socket how and where to process its events
    m_GSIClient->SetEventHandler(*this, GSI_SOCKET_ID);
    m_GSIClient->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    m_GSIClient->Notify(true);

    m_numClients++;
    UpdateStatusBar();
}

/***************************************************************
void GSIServer::OnSocketEvent(wxSocketEvent& event)
****************************************************************/
void GSIServer::OnSocketEvent(wxSocketEvent& event)
{
  //wxString s = _("OnSocketEvent: ");

	try
	{
		wxSocketBase *s = event.GetSocket();
		wxSocketBase &sock=*s;

	  //process the event
		switch(event.GetSocketEvent())
		{
		case wxSOCKET_INPUT:
			{
			// We disable input events, so that the test doesn't trigger
			// wxSocketEvent again.
			sock.SetNotify(wxSOCKET_LOST_FLAG);

			wxInt16 i;
			sock.Read(&i, sizeof(wxInt16));          //send/receive 16 bit integers

			switch (i)
			{
			case GSI_SOCKET_RECORD:
				TextCtrl()->AppendText(_("GSISocketRecord ID\n"));
				GSIServer::ProcessRecordFromClient(sock);
				break;
			case GSI_SOCKET_ASYNC_RECORD:
    			TextCtrl()->AppendText(_("GSISocketAsyncRecord ID\n"));
	    		break;
			case GSI_SOCKET_USER1:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER1 ID\n"));
				ProcessUser1Record(sock);
				break;
			case GSI_SOCKET_USER2:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER2 ID\n"));
				ProcessUser2Record(sock);
				break;
			case GSI_SOCKET_USER3:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER3 ID\n"));
				ProcessUser3Record(sock);
				break;
			case GSI_SOCKET_USER4:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER4 ID\n"));
				ProcessUser4Record(sock);
				break;
			case GSI_SOCKET_USER5:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER5 ID\n"));
				ProcessUser5Record(sock);
				break;
			case GSI_SOCKET_USER6:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER6 ID\n"));
				ProcessUser6Record(sock);
				break;
			case GSI_SOCKET_USER7:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER7 ID\n"));
				ProcessUser7Record(sock);
				break;
			case GSI_SOCKET_USER8:        //A call from a User Client. A StartleClient in this case
					//errorType rv=errNotSupported;
					//rv=ProcessStartleRecord(sock);
				TextCtrl()->AppendText(_("GSI_SOCKET_USER8 ID\n"));
				ProcessUser8Record(sock);
				break;

			default:
    			TextCtrl()->AppendText(_("Unknown test id received from client\n\n"));
				break;
			}

		  // Enable input events again.
			sock.SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
			break;
		}
		case wxSOCKET_LOST:
		{
			m_numClients--;
			m_GSIClient=NULL;
			if(m_numClients==0)
			{
				if(m_GSIFrame)
					m_GSIFrame->SetIcon(wxICON(MONDRIAN));
				for(GSIHandle h=0;h<GSIMaxNumDevices;h++)
					m_GSIFrame->CloseDevice(h);

				//m_GSIFrame->RemoveNode();

			}
			// Destroy() should be used instead of delete wherever possible,
			// due to the fact that wxSocket uses 'delayed events' (see the
			// documentation for wxPostEvent) and we don't want an event to
			// arrive to the event handler (the frame, here) after the socket
			// has been deleted. Also, we might be doing some other thing with
			// the socket at the same time; for example, we might be in the
			// middle of a test or something. Destroy() takes care of all
			// this for us.
			TextCtrl()->AppendText(_("Deleting socket.\n\n"));
			sock.Destroy();
			break;
		 }

		case wxSOCKET_CONNECTION:
			break;
		default:
			break;
		} //event.GetSocketEvent()

		UpdateStatusBar();
	}

	catch(gsException &exc)
	{
        wxString s;
        s.Printf("GSIServer::OnSocketEvent %s. \nError code=%i %s",exc.GetUserString().c_str(),exc.GetErrorCode(),exc.GetString().c_str());
        TextCtrl()->AppendText(s);
	}

}



/***************************************************
void GSIServer::Test1(wxSocketBase *sock)
***************************************************/
void GSIServer::Test1(wxSocketBase *sock)
{
  unsigned char len;
  char *buf;

  //m_text->AppendText(_("Test 1 begins\n"));

  // Receive data from socket and send it back. We will first
  // get a byte with the buffer size, so we can specify the
  // exact size and use the wxSOCKET_WAITALL flag. Also, we
  // disabled input events so we won't have unwanted reentrance.
  // This way we can avoid the infamous wxSOCKET_BLOCK flag.

  sock->SetFlags(wxSOCKET_WAITALL);

  // Read the size
  sock->Read(&len, 1);
  buf = DEBUG_NEW char[len];

  // Read the data
  sock->Read(buf, len);
  //m_text->AppendText(_("Got the data, sending it back\n"));

  // Write it back
  sock->Write(buf, len);
  delete[] buf;

  //m_text->AppendText(_("Test 1 ends\n\n"));
}

/******************************************************
		void GSIServer::Test2(wxSocketBase *sock)
*******************************************************/

void GSIServer::Test2(wxSocketBase *sock)
{
#define MAX_MSG_SIZE 10000

  wxString s;
  wxChar *buf = DEBUG_NEW wxChar[MAX_MSG_SIZE];
  wxUint32 len;

  //m_text->AppendText(_("Test 2 begins\n"));

  // We don't need to set flags because ReadMsg and WriteMsg
  // are not affected by them anyway.

  // Read the message
  len = sock->ReadMsg(buf, MAX_MSG_SIZE * sizeof(wxChar)).LastCount();
  s.Printf(_("Client says: %s\n"), buf);
  //m_text->AppendText(s);
  //m_text->AppendText(_("Sending the data back\n"));

  // Write it back
  sock->WriteMsg(buf, len);
  delete[] buf;

  //m_text->AppendText(_("Test 2 ends\n\n"));

#undef MAX_MSG_SIZE
}


/******************************************************
		void GSIServer::Test3(wxSocketBase *sock)
*******************************************************/
void GSIServer::Test3(wxSocketBase *sock)
{
  unsigned char len;
  char *buf;

  //m_text->AppendText(_("Test 3 begins\n"));

  // This test is similar to the first one, but the len is
  // expressed in kbytes - this tests large data transfers.

  sock->SetFlags(wxSOCKET_WAITALL);

  // Read the size
  sock->Read(&len, 1);
  buf = DEBUG_NEW char[len * 1024];

  // Read the data
  sock->Read(buf, len * 1024);
  //m_text->AppendText(_("Got the data, sending it back\n"));

  // Write it back
  sock->Write(buf, len * 1024);
  delete[] buf;

  //m_text->AppendText(_("Test 3 ends\n\n"));
}




/*!
----------------------------------------------------------------------

void GSIServer::ProcessRecordFromClient(wxSocketBase *sock)

Received a record from the client, called from within the socket handler
No need to hurry as the client will be waiting on a success/fail response
from us
------------------------------------------------------------------------*/

errorType  GSIServer::ProcessRecordFromClient(wxSocketBase &sock)
{

//GSISocketReturnData sr(GSIClient(),GSISocketReturnData:: GSIFail,internal_flag);      //instantiate
    wxString s;
    wxInt16 id;
    errorType rv=errNone;
    TextCtrl()->AppendText("\n\n--------- New GSI Socket Command ----------\n");


    sock.SetFlags(wxSOCKET_WAITALL);
/*--------------Read the id field----------------*/
    sock.Read(&id,sizeof (id));
    if(sock.Error())
    {
        return errSocket;
    }

    s.Printf(_("Type=%i, \n"),id);
    TextCtrl()->AppendText(s);

    switch (id)
    {
    case GSI_COMMAND_OPEN_DEVICE:
		{
			GSIServerCommandOpenDeviceXXX Cmd(sock);

			rv=GetGSIFrame().ProcessServerCommand(Cmd,sock);
			if(rv)
				return rv;
		}
        break;

    case GSI_COMMAND_CLOSE_DEVICE:
		{
			GSIServerCommandCloseDeviceXXX Cmd(sock);
			rv=GetGSIFrame().ProcessServerCommand(Cmd,sock);
			if(rv)
				return rv;
		}
        break;
    case GSI_COMMAND_ADD_NODE:
		{
			GSIServerCommandAddNodeXXX Cmd(sock);
			rv=GetGSIFrame().ProcessServerCommand(Cmd,sock);
			//rv=GetGSIFrame().AddNode(sock);
		if(rv)
			return rv;
		}

        break;
    case GSI_COMMAND_REMOVE_NODE:

		{
			GSIServerCommandRemoveNodeXXX Cmd(sock);
			rv=GetGSIFrame().ProcessServerCommand(Cmd,sock);

		}
    default:
		TextCtrl()->AppendText("Unknown record ID read from socket client");
        break;
    }
    return errNone;

}



/*!
	void	GSIServer::UpdateStatusBar()
*/

void	GSIServer::UpdateStatusBar()
{
#if wxUSE_STATUSBAR
  wxString s;
//GSIStatusBarFrame=0,			//general status
//GSIStatusBarNode,					//node status
//GSIStatusBarDevice,				//Device status
//GSIStatusBarSocket		//socket status

	s.Printf(_("Clients:%d  "), NumClients());
	StatusBar()->SetStatusText(s, GSI_STATUS_BAR_SOCKET);
#endif // wxUSE_STATUSBAR

}

/***********************************************************************
void GSIServer::ProcessUser1Record(wxSocketBase *sock)

Received a record from the client, called from within the socket handler
The id of the record was User1.
These stubs do nothing, it is for the user to override if required
*************************************************************************/

errorType GSIServer::ProcessUser1Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}

errorType GSIServer::ProcessUser2Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}
errorType GSIServer::ProcessUser3Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}
errorType GSIServer::ProcessUser4Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}
errorType GSIServer::ProcessUser5Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}
errorType GSIServer::ProcessUser6Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}
errorType GSIServer::ProcessUser7Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}

errorType GSIServer::ProcessUser8Record(wxSocketBase &sock)
{
	sock;
	return errNone;
}
