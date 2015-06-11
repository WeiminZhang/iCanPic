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

/**************************************************************************************************
client.cpp
Socket client for GSI microcontroller library
This class communicates with the socket server, the server communicates with the microcontroller.
This class sends and receives data through the socket
Data consists of Binary records (see record.hpp), and status information.
The functionality of this class must be provided for the platform that is used to write the front-end
for your application.
This could be as simple as using this class directly in a c++ application as the example code supplied
with this release demonstrates.
It might also require implementing this class as a dll or shared library.
Alternatively, a socket implementation native to your programming language can be used.
Socket.hpp, socket.h,client.hpp,client.h,server.hpp should all be examined if you implement your
own socket client.
***************************************************************************************************/
#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation
    #pragma interface
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
#include "wx/url.h"
#include "wx/wfstream.h"

//#include "gsi-common/include/gsi.hpp"
#include "gsi-common/include/socket.hpp"
#include "client.hpp"
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

//extern wxEVT_GSI_SOCKET_COMMAND;

class GSITestFrame;


void WorkingOpenConnection();

//extern GSITestFrame *g_GSITestFrame;

enum
{
  // menu items
    CLIENT_QUIT = wxID_EXIT,
    CLIENT_ABOUT = wxID_ABOUT,
    CLIENT_OPEN = 100,
    CLIENT_TEST1,
    CLIENT_TEST2,
    CLIENT_TEST3,
    CLIENT_TEST4,
#ifdef STATLE_SERVER
    STARTLE_TEST1,
#endif
    CLIENT_CLOSE,
#if wxUSE_URL
    CLIENT_TESTURL,
#endif
    CLIENT_DGRAM,

  // id for socket
  //SOCKET_ID
};

BEGIN_EVENT_TABLE(GSISocketClient, wxEvtHandler)
		EVT_SOCKET(GSI_SOCKET_ID,     GSISocketClient::OnSocketEvent)
END_EVENT_TABLE()

GSISocketClient::GSISocketClient(wxEvtHandler & evt,wxTextCtrl *text_ctrl)
:m_EventHandler(evt)
{
	m_open=false;

	SetEventHandler(*this, GSI_SOCKET_ID);
  SetNotify(wxSOCKET_CONNECTION_FLAG |
                    wxSOCKET_INPUT_FLAG |
                    wxSOCKET_LOST_FLAG);
  Notify(true);

    m_busy = false;
	//m_EventHandler=*evt;
    m_node=GSINotAValidNode;
    m_TextCtrl=text_ctrl;
}

GSISocketClient::~GSISocketClient()
{
}


#if 0
void GSISocketClient::OnTest(wxCommandEvent& event)
{
	//wxLogMessage(_T("This is the pushed test event handler #%u"), m_level);

        // if we don't skip the event, the other event handlers won't get it:
        // try commenting out this line and see what changes
	event.Skip();
}
#endif



/*********************************************************************************************
void GSISocketClient::OnSocketEvent(wxSocketEvent& event)
Server will send a single msg containg a stream of chars.
First char==id: GSI_SOCKET_ASYNC_RECORD etc
subseqent chars will be the record if this is a GSI_SOCKET_ASYNC_RECORD
Scheme allows us to use this socket as a general ipc mechanism

Because we are unsure what programming language the user will be using
we will not handle the socket msg locally.
Instead we will place it into a GSISocketData class and Post this as an event.
Thus the user code will need to set the parent of the GSISocketClient to the window
that is to handle the event. In the case of this example code, the frame in gsitest.cpp

The user handler will not see wxSOCKET_INPUT events as we will intercept them, sending
instead GSI_SOCKET_ASYNC_RECORD or GSI_SOCKET_RECORD id's
In other words this code could be in a dll?? User code could intercept the PostEvent
and extract the data

There is (should be) a GSSocketClient object for every node we are attached to. The node is
stored in the GSISocketClient.
When posting the message a SockData class is instantiated, the GSISocketClient node is embedded
in the SockData. Ultimately, the top level event handler will see the SockData object.
************************************************************************************************/
void GSISocketClient::OnSocketEvent(wxSocketEvent& event)
{
GSIChar buff[GSIRecordMaxLength+1]; 	//+1 as first byte is an id
bool isRecord=false;

	wxUint32 length=0;
	GSISocketData * SockData =NULL;
	wxSocketBase *sock = event.GetSocket();


  // Now we process the event
    switch(event.GetSocketEvent())
    {
	    case wxSOCKET_INPUT:
	    {
            isRecord=true;

	    // We disable input events, so that the test doesn't trigger
	    // wxSocketEvent again.
		    sock->SetNotify(wxSOCKET_LOST_FLAG);

		    ReadMsg(buff,GSIRecordMaxLength+1); 		  //Get the buffer. +1 as we prepend the GSIRecord with the id GSI_SOCKET_ASYNC_RECORD or whatever
		    length=LastCount();
	    //first element of buffer is the id. Subsequent chars are the data, usually a GSIRecord

	        switch (buff[0])
	        {
			    case GSI_SOCKET_RECORD:
				    //TextCtrl()->AppendText(_("Socket record\n"));
                    AppendText(_("Socket record\n"));
                    SockData= DEBUG_NEW GSISocketData(buff,length,GetNode());
				    break;
			    case GSI_SOCKET_ASYNC_RECORD:
				    //TextCtrl()->AppendText(_("Socket Test record\n"));
                    AppendText(_("Socket Test record\n"));
				    SockData= DEBUG_NEW GSISocketData(buff,length,GetNode());
		    	    break;

			    default:
				    //TextCtrl()->AppendText(_("Unknown socket event\n"));
                    AppendText(_("Unknown socket event\n"));
				    break;
		    //m_text->AppendText(_("Unknown test id received from client\n\n"));
            }

        // Enable input events again.
        sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
	    break;
	    }
    case wxSOCKET_LOST:
	{
 	    //TextCtrl()->AppendText(_("Socket lost\n"));
        AppendText(_("Socket lost\n"));

	  //m_numClients--;

	  // Destroy() should be used instead of delete wherever possible,
	  // due to the fact that wxSocket uses 'delayed events' (see the
	  // documentation for wxPostEvent) and we don't want an event to
	  // arrive to the event handler (the frame, here) after the socket
	  // has been deleted. Also, we might be doing some other thing with
	  // the socket at the same time; for example, we might be in the
	  // middle of a test or something. Destroy() takes care of all
	  // this for us.

	  //TextCtrl()->AppendText(_("Deleting socket.\n\n"));

			//EVT_MENU(wxTheApp, CLIENT_CLOSE);
			//wxCommandEvent event(WXTYPE commandEventType = 0, int id = 0)

//This calls the Close socket menu item directly
//This should be done without using knowledge of the menu structure
//We will use a custom event with the type of socket event and any GSIRecord
//the eventHandler set by the parent can then handle any details.

//This function will also be called if we attempt to connect to a non-existent server, ie. not currently open
#warning message("Work required here")
		//wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, CLIENT_CLOSE);
		AppendText(_("Deleting socket.\n\n"));
        if(IsOpen()==false)
        {
            //not a close received from the server, probably a failed connection

             return;
        }
//Don't destroy yet
        m_open=false;
//Mustn't throw as it will not be caught in our app
        //GSI_THROW("SocketLost",errFail);

        //wxPostEvent((wxEvtHandler *)g_GSITestFrame,event);
        //sock->Destroy();

		//wxSocketEvent evt(wxSOCKET_LOST);
		//wxPostEvent(&m_EventHandler, evt);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
		int n=GetNode();
		n;
		//SockData= DEBUG_NEW GSISocketData(buff,length,GSI_NODE_THIS);
        wxGSISocketCommandEvent eventCustom(wxEVT_GSI_SOCKET_COMMAND);          //will go to GSIClientTestFrame
        eventCustom.SetSockData(SockData);
        eventCustom.SetClientData(SockData);
        eventCustom.SetClientObject((wxClientData *)sock);
        eventCustom.SetSocketBase(sock);
		eventCustom.SetId(GSI_SOCKET_LOST);
	    wxPostEvent(&m_EventHandler, eventCustom);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)

        return;
	}
	case wxSOCKET_CONNECTION:
		SockData= DEBUG_NEW GSISocketData(GSI_SOCKET_CONNECTION);
        m_open=true;
		break;

	default:
        wxASSERT(0);
        return;
  }
#warning ("XXXXX")
#if 0
    wxGSISocketCommandEvent customEvent(wxEVT_GSI_SOCKET_COMMAND);
    customEvent.SetEventObject(this);
    customEvent.SetClientObject((wxClientData *)SockData);
    //GetEventHandler()->ProcessEvent(customEvent);
    wxPostEvent(m_EventHandler, customEvent);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
#endif
#if 0
	wxCommandEvent eventCustom(wxEVT_GSI_SOCKET);

	//GSIRecord *r=new GSIRecord (record); //the event handler will delete the object
	eventCustom.SetClientObject((wxClientData *)SockData);
	wxPostEvent(m_EventHandler, eventCustom);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
#endif

	//wxCommandEvent eventCustom(wxEVT_GSI_SOCKET_COMMAND);
    //wxSocketEvent eventCustom(wxEVT_GSI_SOCKET_COMMAND);

	//GSIRecord *r=new GSIRecord (record); //the event handler will delete the object
	//eventCustom.SetClientObject((wxClientData *)SockData);
    //eventCustom.SetClientObject((wxClientData *)SockData);

#ifndef DEBUG_GSI_SOCKET_EVENT
    //wxGSISocketCommandEvent *eventCustom;

    //eventCustom=DEBUG_NEW wxGSISocketCommandEvent(wxEVT_GSI_SOCKET_COMMAND);
        //wxGSISocketCommandEvent eventCustom();
    if( isRecord)
    {
        wxGSISocketCommandEvent eventCustom(wxEVT_GSI_SOCKET_RECORD_RECEIVED); //will go to GSIClientNBPanel
		//wxGSISocketCommandEvent eventCustom(wxEVT_GSI_SOCKET_COMMAND);          //will go to GSIClientTestFrame
        eventCustom.SetSockData(SockData);
        eventCustom.SetClientData(SockData);
        eventCustom.SetClientObject((wxClientData *)sock);
        eventCustom.SetSocketBase(sock);
	    wxPostEvent(&m_EventHandler, eventCustom);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
    }
    else
    {
        wxGSISocketCommandEvent eventCustom(wxEVT_GSI_SOCKET_COMMAND);          //will go to GSIClientTestFrame
        eventCustom.SetSockData(SockData);
        eventCustom.SetClientData(SockData);
        eventCustom.SetClientObject((wxClientData *)sock);
        eventCustom.SetSocketBase(sock);
	    wxPostEvent(&m_EventHandler, eventCustom);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
    }
#endif
  //UpdateStatusBar();
}


errorType GSISocketClient::ConnectToServer(int sock_address, const wxString & host)
{

wxIPV4address addr;

    // Ask user for server address
    addr.Hostname(host);
    addr.Service((unsigned short)sock_address);

    wxSocketClient::Connect(addr, false);
    WaitOnConnect(10);

    if (IsConnected())
	{
	    m_open=true;
	}
    else
    {
        Close();
        wxMessageBox(_("Can't connect to the specified host"), _("Alert !"));
        return errFail;
    }
    return errNone;
}



/*!
 * GSISocketClient::AppendText(const wxString &text)
 */
void GSISocketClient::AppendText(const wxString &text)
{

    if(m_TextCtrl)
        m_TextCtrl->AppendText(text);
}

#if DIALOG_BLOCKS

/*****************************************************************************************
					ctor
GSITestClient::GSITestClient(wxStatusBar *status_bar,wxTextCtrl *text_ctrl)
*****************************************************************************************/
//GSITestClient::GSITestClient(wxEvtHandler * evt,GSISocketClientStatusBar *status_bar,wxTextCtrl *text_ctrl)
//:GSISocketClient(evt)
GSITestClient::GSITestClient(wxEvtHandler & evt_handler,wxTextCtrl *text_ctrl)
:GSISocketClient(evt_handler)
{
	m_TextCtrl=text_ctrl;
}


#else
/*****************************************************************************************
					ctor
GSITestClient::GSITestClient(wxStatusBar *status_bar,wxTextCtrl *text_ctrl)
*****************************************************************************************/
//GSITestClient::GSITestClient(wxEvtHandler * evt,GSISocketClientStatusBar *status_bar,wxTextCtrl *text_ctrl)
//:GSISocketClient(evt)
GSITestClient::GSITestClient(GSITestFrame * gsi_test_frame,GSISocketClientStatusBar *status_bar,wxTextCtrl *text_ctrl)
:GSISocketClient(gsi_test_frame)
{
	m_TextCtrl=text_ctrl;
	m_StatusBar=status_bar;
    m_GSITestFrame=gsi_test_frame;
}

#endif //DIALOG_BLOCKS


errorType GSISocketClient::CloseSocket()
{
GSISocketData * SockData =NULL;
    Close();
    m_open=false;
//Now notify the GSISocketClientTest class if there is one
#warning ("This event has probably already been sent by the socket event handler")
    SockData= DEBUG_NEW GSISocketData(GSI_SOCKET_LOST);       //this will be deleted by the handler, what if there isn't one?!
#ifndef DEBUG_GSI_SOCKET_EVENT
	wxCommandEvent eventCustom(wxEVT_GSI_SOCKET_COMMAND);

	eventCustom.SetClientObject((wxClientData *)SockData);
	wxPostEvent(&m_EventHandler, eventCustom);       //handled by: GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
#endif
    return errNone;
}


void GSISocketClient::SetNodeHandle(GSINodeHandle node)
{


    wxASSERT_MSG(node >=0 && node < GSIMaxNumNodeHandles,"node out of range");
    m_node=node;
    if(node <0 || node >= GSIMaxNumNodeHandles)
        m_node=GSINotAValidNodeHandle;
}
