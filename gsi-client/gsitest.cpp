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
#include "wx/notebook.h"

//#include <eh.h>

#include "wx/socket.h"
#include "wx/url.h"
#include "wx/wfstream.h"
#include "gsi-common/include/socket.hpp"
#include "client.hpp"
#include "lcdClient.hpp"

#ifdef STARTLE_SERVER
#include "../gsi-server/socket/startle.hpp"
#endif

#include "gsitest.hpp"



#if DIALOG_BLOCKS
#else

// --------------------------------------------------------------------------
// resources
// --------------------------------------------------------------------------

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif


// the application icon
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "mondrian.xpm"
#endif

//GSITestFrame *g_GSITestFrame;

 // --------------------------------------------------------------------------
// constants
// --------------------------------------------------------------------------


DEFINE_EVENT_TYPE(wxEVT_GSI_SOCKET)

// --------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// --------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GSITestFrame, wxFrame)
    EVT_MENU(CLIENT_QUIT,     GSITestFrame::OnQuit)
    EVT_MENU(CLIENT_ABOUT,    GSITestFrame::OnAbout)
    EVT_MENU(CLIENT_OPEN,     GSITestFrame::OnOpenConnection)
    EVT_MENU(CLIENT_TEST1,    GSITestFrame::OnTest1)
    EVT_MENU(CLIENT_TEST2,    GSITestFrame::OnTest2)
    EVT_MENU(CLIENT_TEST3,    GSITestFrame::OnTest3)
    EVT_MENU(CLIENT_TEST4,    GSITestFrame::OnTest4)
    EVT_MENU(CLIENT_CLOSE,    GSITestFrame::OnCloseConnection)
    EVT_MENU(CLIENT_DGRAM,    GSITestFrame::OnDatagram)
    EVT_MENU(CLIENT_DEBUG,    GSITestFrame::OnDebugSocket)
    //EVT_SOCKET(GSISocketID,   GSITestFrame::OnSocketEvent)

    EVT_GSI_SOCKET(wxID_ANY, GSITestFrame::OnGSISocketEvent)			//custom event from GSISocketClient
#if wxUSE_URL
    EVT_MENU(CLIENT_TESTURL,  GSITestFrame::OnTestURL)
#endif
END_EVENT_TABLE()



// ==========================================================================
// implementation
// ==========================================================================





// --------------------------------------------------------------------------
// main frame
// --------------------------------------------------------------------------

/***************************************************************************************
        ctor
GSITestFrame::GSITestFrame()

***************************************************************************************/

// frame constructor
GSITestFrame::GSITestFrame() : wxFrame((wxFrame *)NULL, wxID_ANY,
                             _("GSITest sock Client"),
                             wxDefaultPosition, wxSize(600, 600))
{
    try
    {
      // Give the frame an icon
        SetIcon(wxICON(MONDRIAN));

        // Make menus
        m_menuFile = DEBUG_NEW wxMenu();
        m_menuFile->Append(CLIENT_ABOUT, _("&About...\tCtrl-A"), _("Show about dialog"));
        m_menuFile->AppendSeparator();
        m_menuFile->Append(CLIENT_QUIT, _("E&xit\tAlt-X"), _("Quit client"));

        m_menuSocket = DEBUG_NEW wxMenu();
        m_menuSocket->Append(CLIENT_OPEN, _("&Open session"), _("Connect to server"));
        m_menuSocket->AppendSeparator();
        m_menuSocket->Append(CLIENT_TEST1, _("Test &1"), _("Test basic functionality"));
        m_menuSocket->Append(CLIENT_TEST2, _("Test &2"), _("Test ReadMsg and WriteMsg"));
        m_menuSocket->Append(CLIENT_TEST3, _("Test &3"), _("Test large data transfer"));
        m_menuSocket->AppendSeparator();

        m_menuSocket->Append(CLIENT_TEST4, _("Test &GSI"), _("Test record transfer"));
        m_menuSocket->AppendSeparator();


        m_menuSocket->Append(CLIENT_CLOSE, _("&Close session"), _("Close connection"));

        m_menuDatagramSocket = DEBUG_NEW wxMenu();
        m_menuDatagramSocket->Append(CLIENT_DGRAM, _("Send Datagram"), _("Test UDP sockets"));

        m_menuDebugSocket = DEBUG_NEW wxMenu();
        m_menuDebugSocket->Append(CLIENT_DEBUG, _("Debug"), _("Test routine"));


    #if wxUSE_URL
        m_menuProtocols = DEBUG_NEW wxMenu();
        m_menuProtocols->Append(CLIENT_TESTURL, _("Test URL"), _("Get data from the specified URL"));
    #endif

      // Append menus to the menubar
        m_menuBar = DEBUG_NEW wxMenuBar();
        m_menuBar->Append(m_menuFile, _("&File"));
        m_menuBar->Append(m_menuSocket, _("&SocketClient"));
        m_menuBar->Append(m_menuDatagramSocket, _("&DatagramSocket"));
        m_menuBar->Append(m_menuDebugSocket, _("&DebugSocket"));

    #if wxUSE_URL
        m_menuBar->Append(m_menuProtocols, _("&Protocols"));
    #endif
        SetMenuBar(m_menuBar);

    #if wxUSE_STATUSBAR
      // Status bar
	    m_StatusBar=DEBUG_NEW GSISocketClientStatusBar(this,-1);
	    SetStatusBar(m_StatusBar);			//associate bar with frame. This lets Frame manage the bar
    #endif // wxUSE_STATUSBAR

      // Make a textctrl for logging
        //m_Panel=DEBUG_NEW wxPanel(this,10,10,500,500);
        //m_Panel=DEBUG_NEW wxPanel(this);


        wxPanel *panel;
        panel=DEBUG_NEW wxPanel(this);
        wxSize  sz(200,400);
#if 0
        m_TextCtrl  = DEBUG_NEW wxTextCtrl(panel, wxID_ANY,
                               _("Welcome to wxSocket demo: Client\nClient ready\n"),
                               wxDefaultPosition, sz,           //wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY);
#else
        m_TextCtrl  = DEBUG_NEW wxTextCtrl(panel, wxID_ANY,
                               _("Welcome to wxSocket demo: Client\nClient ready\n"),
                               wxPoint(0,0), sz,           //wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY);
        wxString choices[] =
        {
            _T("This"),
            _T("is one of my"),
            _T("really"),
            _T("wonderful"),
            _T("examples.")
        };

        wxListBox * lb = new wxListBox( panel, wxID_ANY,
                               wxPoint(220,0), wxSize(120,70),
                               5, choices, wxLB_MULTIPLE |wxLB_ALWAYS_SB );

        wxBookCtrl *book= DEBUG_NEW wxBookCtrl(panel,wxID_ANY,
        wxPoint(350,0),wxSize(200,350));


        wxPanel *panel1 =DEBUG_NEW wxPanel(book);
        book->AddPage(panel1,"Page1");

        panel1 =DEBUG_NEW wxPanel(book);
        book->AddPage(panel1,"Page2");
#endif

#if 0
        wxPanel *panel2;
        panel2=DEBUG_NEW wxPanel(this);
        wxBookCtrl *book= DEBUG_NEW wxBookCtrl(panel2,wxID_ANY,
        wxPoint(200,0),wxSize(100,200));


        wxPanel *panel1 =DEBUG_NEW wxPanel(book);
        book->AddPage(panel1,"Test");
        panel1 =DEBUG_NEW wxPanel(book);
        book->AddPage(panel1,"Test");

#endif


      // Create the socket

	    m_Client=DEBUG_NEW GSITestClient(this,m_StatusBar,TextCtrl());
        if(m_Client == NULL)
            GSI_THROW("Couldn't allocate socket object",errMem);

	    m_busy = false;
        m_StatusBar->Update();
        //GSI_THROW("HELLO!",errFail);            //works ok


#if 0
        m_Panel = DEBUG_NEW GSITestPanel( this, 10, 10, 300, 100 );
        SetSizeHints( 500, 425 );




        m_Notebook=DEBUG_NEW wxNotebook(this,wxID_ANY,wxDefaultPosition,wxSize(200,200));

        wxNotebookPage * Page1 = new wxNotebookPage(m_Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTAB_TRAVERSAL | wxCLIP_CHILDREN | wxNO_BORDER | wxNO_FULL_REPAINT_ON_RESIZE);

        //wxPanel * window1=DEBUG_NEW wxPanel(m_Notebook,wxID_ANY);
        m_Notebook->AddPage(Page1,_("Tab one"),true,0);
        m_Notebook->Show();
#endif

#if 0
        m_sizerFrame = DEBUG_NEW wxBoxSizer(wxVERTICAL);

        m_sizerFrame->Add(m_TextCtrl, 1, wxEXPAND);

        Page1->SetSizer(m_sizerFrame);

        m_sizerFrame->Fit(this);
        m_sizerFrame->SetSizeHints(this);
#endif
    }
    catch (gsException &exc)
    {
        wxString s;
        s.Printf("Exception: %s. Error code=%i",exc.GetUserString(),exc.GetErrorType());
        TextCtrl()->AppendText(s);
        wxMessageBox(_(s),
               _("Error"),
               wxOK | wxICON_EXCLAMATION, this);
    }
    catch(...)
    {
         wxMessageBox(_("Unhandled exception"),
               _("Error"),
               wxOK | wxICON_EXCLAMATION, this);
    }
}

/***************************************************************************************
        dtor
GSITestFrame::~GSITestFrame()

***************************************************************************************/
GSITestFrame::~GSITestFrame()
{
  // No delayed deletion here, as the frame is dying anyway
  //delete m_Client->m_Sock;
	//if (m_Client)
	//		delete m_Client;

}

/************************************************************
bool GSITestFrame::ProcessEvent(wxEvent& event)
needed so that we can catch exceptions in event handlers
************************************************************/
bool GSITestFrame::ProcessEvent(wxEvent& event)
{
    try
    {
        return wxFrame::ProcessEvent(event);
    }
    catch (GSISocketException &exc)
    {
        //exc.GetSock();
        CloseConnection();
        wxString s;
        s.Printf("Exception: %s. Error code=%i",exc.GetUserString(),exc.GetErrorType());
        TextCtrl()->AppendText(s);
        wxMessageBox(_(s),
               _("Error"),
               wxOK | wxICON_EXCLAMATION, this);
        return(true);
    }

    catch ( ...)
    {

        return true;
    }
}

// event handlers

void GSITestFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
  // true is to force the frame to close
  Close(true);
}

void GSITestFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxMessageBox(_("GSI socket client (c) 2006 Glenn Self\n"),
               _("About Client"),
               wxOK | wxICON_INFORMATION, this);
}


void GSITestFrame::OnOpenConnection(wxCommandEvent& WXUNUSED(event))
{
	m_Client->ConnectToServer(3000);

  // Mini-tutorial for Connect() :-)
  // ---------------------------
  //
  // There are two ways to use Connect(): blocking and non-blocking,
  // depending on the value passed as the 'wait' (2nd) parameter.
  //
  // Connect(addr, true) will wait until the connection completes,
  // returning true on success and false on failure. This call blocks
  // the GUI (this might be changed in future releases to honour the
  // wxSOCKET_BLOCK flag).
  //
  // Connect(addr, false) will issue a nonblocking connection request
  // and return immediately. If the return value is true, then the
  // connection has been already successfully established. If it is
  // false, you must wait for the request to complete, either with
  // WaitOnConnect() or by watching wxSOCKET_CONNECTION / LOST
  // events (please read the documentation).
  //
  // WaitOnConnect() itself never blocks the GUI (this might change
  // in the future to honour the wxSOCKET_BLOCK flag). This call will
  // return false on timeout, or true if the connection request
  // completes, which in turn might mean:
  //
  //   a) That the connection was successfully established
  //   b) That the connection request failed (for example, because
  //      it was refused by the peer.
  //
  // Use IsConnected() to distinguish between these two.
  //
  // So, in a brief, you should do one of the following things:
  //
  // For blocking Connect:
  //
  //   bool success = client->Connect(addr, true);
  //
  // For nonblocking Connect:
  //
  //   client->Connect(addr, false);
  //
  //   bool waitmore = true;
  //   while (! client->WaitOnConnect(seconds, millis) && waitmore )
  //   {
  //     // possibly give some feedback to the user,
  //     // update waitmore if needed.
  //   }
  //   bool success = client->IsConnected();
  //
  // And that's all :-)

  m_StatusBar->Update();

}

void GSITestFrame::OnTest1(wxCommandEvent& WXUNUSED(event))
{

  const wxChar *buf1;
  wxChar       *buf2;
  unsigned char len;

  // Disable socket menu entries (exception: Close Session)
    m_busy = true;
	m_StatusBar->Update();

  TextCtrl()->AppendText(_("\n=== Test 1 begins ===\n"));

  // Tell the server which test we are running
  unsigned char c = 0xBE;
  m_Client->Write(&c, 1);

  // Send some data and read it back. We know the size of the
  // buffer, so we can specify the exact number of bytes to be
  // sent or received and use the wxSOCKET_WAITALL flag. Also,
  // we have disabled menu entries which could interfere with
  // the test, so we can safely avoid the wxSOCKET_BLOCK flag.
  //
  // First we send a byte with the length of the string, then
  // we send the string itself (do NOT try to send any integral
  // value larger than a byte "as is" across the network, or
  // you might be in trouble! Ever heard about big and little
  // endian computers?)

  m_Client->SetFlags(wxSOCKET_WAITALL);

  buf1 = _("Test string (less than 256 chars!)");
  len  = (unsigned char)((wxStrlen(buf1) + 1) * sizeof(wxChar));
  buf2 = DEBUG_NEW wxChar[wxStrlen(buf1) + 1];

  TextCtrl()->AppendText(_("Sending a test buffer to the server ..."));
  m_Client->Write(&len, 1);
  m_Client->Write(buf1, len);
  TextCtrl()->AppendText(m_Client->Error() ? _("failed !\n") : _("done\n"));

  TextCtrl()->AppendText(_("Receiving the buffer back from server ..."));
  m_Client->Read(buf2, len);
  TextCtrl()->AppendText(m_Client->Error() ? _("failed !\n") : _("done\n"));

  TextCtrl()->AppendText(_("Comparing the two buffers ..."));
  if (memcmp(buf1, buf2, len) != 0)
  {
    TextCtrl()->AppendText(_("failed!\n"));
    TextCtrl()->AppendText(_("Test 1 failed !\n"));
  }
  else
  {
    TextCtrl()->AppendText(_("done\n"));
    TextCtrl()->AppendText(_("Test 1 passed !\n"));
  }
  TextCtrl()->AppendText(_("=== Test 1 ends ===\n"));

  delete[] buf2;
  m_busy = false;
  m_StatusBar->Update();
}

void GSITestFrame::OnTest2(wxCommandEvent& WXUNUSED(event))
{

  const wxChar *msg1;
  wxChar *msg2;
  size_t len;

  // Disable socket menu entries (exception: Close Session)
  m_busy = true;
  m_StatusBar->Update();

  TextCtrl()->AppendText(_("\n=== Test 2 begins ===\n"));

  // Tell the server which test we are running
  unsigned char c = 0xCE;
  m_Client->Write(&c, 1);

  // Here we use ReadMsg and WriteMsg to send messages with
  // a header with size information. Also, the reception is
  // event triggered, so we test input events as well.
  //
  // We need to set no flags here (ReadMsg and WriteMsg are
  // not affected by flags)

  m_Client->SetFlags(wxSOCKET_WAITALL);

  wxString s = wxGetTextFromUser(
    _("Enter an arbitrary string to send to the server:"),
    _("Test 2 ..."),
    _("Yes I like wxWidgets!"));

  msg1 = s.c_str();
  len  = (wxStrlen(msg1) + 1) * sizeof(wxChar);
  msg2 = DEBUG_NEW wxChar[wxStrlen(msg1) + 1];

  TextCtrl()->AppendText(_("Sending the string with WriteMsg ..."));
  m_Client->WriteMsg(msg1, len);
  TextCtrl()->AppendText(m_Client->Error() ? _("failed !\n") : _("done\n"));
  TextCtrl()->AppendText(_("Waiting for an event (timeout = 2 sec)\n"));

  // Wait until data available (will also return if the connection is lost)
  m_Client->WaitForRead(2);

  if (m_Client->IsData())
  {
    TextCtrl()->AppendText(_("Reading the string back with ReadMsg ..."));
    m_Client->ReadMsg(msg2, len);
    TextCtrl()->AppendText(m_Client->Error() ? _("failed !\n") : _("done\n"));
    TextCtrl()->AppendText(_("Comparing the two buffers ..."));
    if (memcmp(msg1, msg2, len) != 0)
    {
      TextCtrl()->AppendText(_("failed!\n"));
      TextCtrl()->AppendText(_("Test 2 failed !\n"));
    }
    else
    {
      TextCtrl()->AppendText(_("done\n"));
      TextCtrl()->AppendText(_("Test 2 passed !\n"));
    }
  }
  else
    TextCtrl()->AppendText(_("Timeout ! Test 2 failed.\n"));

  TextCtrl()->AppendText(_("=== Test 2 ends ===\n"));

  delete[] msg2;
  m_busy = false;
  m_StatusBar->Update();
}


void GSITestFrame::OnTest3(wxCommandEvent& WXUNUSED(event))
{
  char *buf1;
  char *buf2;
  unsigned char len;

  // Disable socket menu entries (exception: Close Session)
  m_busy = true;
  m_StatusBar->Update();

  TextCtrl()->AppendText(_("\n=== Test 3 begins ===\n"));

  // Tell the server which test we are running
  unsigned char c = 0xDE;
  m_Client->Write(&c, 1);

  // This test also is similar to the first one but it sends a
  // large buffer so that wxSocket is actually forced to split
  // it into pieces and take care of sending everything before
  // returning.

  m_Client->SetFlags(wxSOCKET_WAITALL);

  // Note that len is in kbytes here!
  len  = 32;
  buf1 = DEBUG_NEW char[len * 1024];
  buf2 = DEBUG_NEW char[len * 1024];

  for (int i = 0; i < len * 1024; i ++)
    buf1[i] = (char)(i % 256);

  TextCtrl()->AppendText(_("Sending a large buffer (32K) to the server ..."));
  m_Client->Write(&len, 1);

  m_Client->Write(buf1, len * 1024);
  TextCtrl()->AppendText(m_Client->Error() ? _("failed !\n") : _("done\n"));

  TextCtrl()->AppendText(_("Receiving the buffer back from server ..."));
  m_Client->Read(buf2, len * 1024);
  TextCtrl()->AppendText(m_Client->Error() ? _("failed !\n") : _("done\n"));

  TextCtrl()->AppendText(_("Comparing the two buffers ..."));
  if (memcmp(buf1, buf2, len) != 0)
  {
    TextCtrl()->AppendText(_("failed!\n"));
    TextCtrl()->AppendText(_("Test 3 failed !\n"));
  }
  else
  {
    TextCtrl()->AppendText(_("done\n"));
    TextCtrl()->AppendText(_("Test 3 passed !\n"));
  }
  TextCtrl()->AppendText(_("=== Test 3 ends ===\n"));

  delete[] buf2;
  m_busy = false;
  m_StatusBar->Update();
}


void GSITestFrame::OnTest4(wxCommandEvent& WXUNUSED(event))
{
    //GSISocketClient			 * const c=new GSITestClient(this,m_StatusBar,TextCtrl());

    //c=new GSITestClient(this,m_StatusBar,TextCtrl());
GSIHandle hNode;
GSIHandle hDevice;
bool isOpen=false;

    try
    {
    //open device would usually be called on initialisation as it is node0 or at least the node thought which we
    //are communicating to the CAN.
    //AddNode() would cause a new notebook page to be added
    GSICommandOpenDevice openCmd(GetGSISocketClient(),GetTextCtrl(),"Com2");
    wxString s;
    int n=openCmd.GetReturnNumInts();
    if(n==2)                //should always be one, the node we are attached to
    {
        const wxInt16 *data=openCmd.GetReturnData();
        hDevice=data[0];
        hNode=data[1];
        s.Printf(_("node %i is attached to device handle %i\n"),hNode,hDevice);
        isOpen=true;

    }
    else
    {
        s="No Node found\n";
        return;
    }
    TextCtrl()->AppendText(s);

    if(isOpen)
    {
        GSICommandAddNode(GetGSISocketClient(),GetTextCtrl(),1,hDevice); //allow us to talk to node1


        GSICommandRemoveNode remove(GetGSISocketClient(),GetTextCtrl(),1);
        int n=openCmd.GetReturnNumInts();      //should be 0, no data returned
    }
    }
    catch(...)
    {
        int i=0;
    }


}

/*************************************************************************************
void GSITestFrame::OnCloseConnection(wxCommandEvent& WXUNUSED(event))
*************************************************************************************/
void GSITestFrame::OnCloseConnection(wxCommandEvent& WXUNUSED(event))
{
//This can be called from a socket event on a refusal to create a socket.
//In this case m_Client==NULL if we have not got a socket already open
    CloseConnection();
}

/*************************************************************************************
void GSITestFrame::CloseSocket()
*************************************************************************************/

void GSITestFrame::CloseConnection()
{
	if (m_Client)
	{
		m_menuSocket->Enable(CLIENT_OPEN, false);
		m_menuSocket->Enable(CLIENT_CLOSE, false);

    	m_Client->Close();
		//m_Client=NULL;
		m_StatusBar->Update();
        //GSI_THROW("SocketLost",errFail);
	}

}


void GSITestFrame::OnDatagram(wxCommandEvent& WXUNUSED(event))
{
  TextCtrl()->AppendText(_("\n=== Datagram test begins ===\n"));
  TextCtrl()->AppendText(_("Sorry, not implemented\n"));
  TextCtrl()->AppendText(_("=== Datagram test ends ===\n"));
}

#if wxUSE_URL

void GSITestFrame::OnTestURL(wxCommandEvent& WXUNUSED(event))
{
  // Note that we are creating a new socket here, so this
  // won't mess with the client/server demo.

  // Ask for the URL
  TextCtrl()->AppendText(_("\n=== URL test begins ===\n"));
  wxString urlname = wxGetTextFromUser(_("Enter an URL to get"),
                                       _("URL:"),
                                       _T("http://localhost"));

  // Parse the URL
  wxURL url(urlname);
  if (url.GetError() != wxURL_NOERR)
  {
    TextCtrl()->AppendText(_("Error: couldn't parse URL\n"));
    TextCtrl()->AppendText(_("=== URL test ends ===\n"));
    return;
  }

  // Try to get the input stream (connects to the given URL)
  TextCtrl()->AppendText(_("Trying to establish connection...\n"));
  wxYield();
  wxInputStream *data = url.GetInputStream();
  if (!data)
  {
    TextCtrl()->AppendText(_("Error: couldn't read from URL\n"));
    TextCtrl()->AppendText(_("=== URL test ends ===\n"));
    return;
  }

  // Print the contents type and file size
  wxString s;
  s.Printf(_("Contents type: %s\nFile size: %i\nStarting to download...\n"),
             url.GetProtocol().GetContentType().c_str(),
             data->GetSize());
  TextCtrl()->AppendText(s);
  wxYield();

  // Get the data
  wxFile fileTest(wxT("test.url"), wxFile::write);
  wxFileOutputStream sout(fileTest);
  if (!sout.Ok())
  {
    TextCtrl()->AppendText(_("Error: couldn't open file for output\n"));
    TextCtrl()->AppendText(_("=== URL test ends ===\n"));
    return;
  }

  data->Read(sout);
  TextCtrl()->AppendText(_("Results written to file: test.url\n"));
  TextCtrl()->AppendText(_("Done.\n"));
  TextCtrl()->AppendText(_("=== URL test ends ===\n"));

  delete data;
}

#endif



/**********************************************************************************
void GSIFrame::OnGSISocketEvent(wxCommandEvent& event)
The server has sent socket data to the client which has sent this event.
The data is sent as a simple char buffer.

***********************************************************************************/
void GSITestFrame::OnGSISocketEvent(wxCommandEvent& event)
{

	GSISocketData *Sock_Data;
	Sock_Data=(GSISocketData *) event.GetClientObject();
    GSIRecord * r=DEBUG_NEW GSIRecord;

    int id=Sock_Data->GetID();
	switch (id )
	{
		case GSISocketInput:
            TextCtrl()->AppendText(_("GSISocketInput\n"));
            break;
        case GSISocketOutput:
            TextCtrl()->AppendText(_("GSISocketOutput\n"));
            break;
        case GSISocketConnection:
            TextCtrl()->AppendText(_("GSISocketConnection\n"));
            SetIcon(wxICON(CONN_ICN));
            break;
		case GSISocketLost:     //This doesn't ever appear to be called
            TextCtrl()->AppendText(_("GSISocketLost\n"));
            SetIcon(wxICON(MONDRIAN));
			break;
        case GSISocketRecord:
            TextCtrl()->AppendText(_("GSISocketRecord\n"));
            break;
        case GSISocketAsyncRecord:
            {
                if (r->SocketDataToRecord(*Sock_Data))
                {
                    //error
                    TextCtrl()->AppendText(_("GSISocketAsyncRecord: ERROR reading record!\n"));
                }
                else
                {
                    //GSIStringCANFrame sf;
                    wxString s;
                     // \ is continuation character
                    s.Printf(_("GSISocketAsyncRecord:\n"
                    "-----------------------------------------\n"
                    "Type=%s\nNode=%s\nSubfunction=%s\nData=%s\n"
                    "------------------------------------------\n"),
                    r->GetTypeString(),
                    r->GetNodeString(),
                    r->GetSubfuncString(),
                    r->GetDataString()                    );

                    TextCtrl()->AppendText(s);
                    if(r->GetType()==rtGSIR2RxCANMessageNode2PC)
                    {
                    #if 1
                        GSICANFrame &frame=r->GetGSICANFrame();
                        s.Printf(_("CAN frame received:\n"
                        "CAN id=%s\n"
                        "Origin  Node=%s\n"
                        "subfunction=%s\n"
                        "%s%s"),                      //two optional information strings, null or /n terminated
                        frame.GetIDString(),
                        frame.GetOriginNodeString(),
                        frame.GetSubfuncString(),
                        frame.GetInformationString1(),
                        frame.GetInformationString2()
                        );

                        TextCtrl()->AppendText(s);
                    #endif

                    }
                }
            }

            break;
		default:
			break;
	}

	if (Sock_Data)			//Usually a GSIRecord
		delete Sock_Data;
    delete r;

}


/***********************************************************************
					ctor
			GSISocketClientStatusBar
*************************************************************************/
GSISocketClientStatusBar::GSISocketClientStatusBar(GSITestFrame *gsi_frame,wxWindowID id)
:wxStatusBar(gsi_frame,id),m_GSITestFrame(gsi_frame)
{

	SetFieldsCount(2);

}

GSISocketClientStatusBar::~GSISocketClientStatusBar()
{
}

void GSISocketClientStatusBar::Update()
{
	wxString s;
	bool connected=false;

	s.Printf(_("Not connected"));

	//if( m_GSITestFrame->GetGSISocketClient())       //guaranteed to exist if class exists
	//{
    if( m_GSITestFrame->GetGSISocketClient().IsConnected())
    {
	    connected=true;
	    wxIPV4address addr;

	    m_GSITestFrame->GetGSISocketClient().GetPeer(addr);
	    s.Printf(_("%s : %d"), (addr.Hostname()).c_str(), addr.Service());
        m_GSITestFrame->SetIcon(wxICON(CONN_ICN));

    }
    else
        m_GSITestFrame->SetIcon(wxICON(MONDRIAN));
    //}

#if wxUSE_STATUSBAR
  SetStatusText(s, 1);
#endif // wxUSE_STATUSBAR

	m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_OPEN, ! connected && ! m_GSITestFrame->GetBusyFlag());

    m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_OPEN, ! connected && !m_GSITestFrame->GetBusyFlag());
    m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_TEST1, connected && !m_GSITestFrame->GetBusyFlag());
    m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_TEST2, connected && !m_GSITestFrame->GetBusyFlag());
    m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_TEST3, connected && !m_GSITestFrame->GetBusyFlag());
    m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_TEST4, connected && !m_GSITestFrame->GetBusyFlag());
    m_GSITestFrame->GetSocketMenu()->Enable(CLIENT_CLOSE, connected);

}





void GSITestFrame::OnDebugSocket(wxCommandEvent& WXUNUSED(event))
{
    GSIRecord r;
    errorType rv=errNone;
    rv=rv;

    //int error=0;
#if 0
//simulate a key press of key 8 on CAN slave (node 1), sent to node 0 the CAN master attached to this pc

    //mimic the data sent by the pic
    //When setting data, should set the length or else the strings will not be set
    r.SetLength(8);          //no special significance to length of 8, max length ==14
    error+=r.SetData(0,228);       //id of CAN node==4068 node0 (CAN_GSI_BASE+4)
    error+=r.SetData(1,15);        //2nd byte id
    error+=r.SetData(2,0);         //3rd byte id
    error+=r.SetData(3,0);         //4th byte id
    error+=r.SetData(4,1);         //node
    error+=r.SetData(5,1);         //subfunc
    error+=r.SetData(6,56);        //data
    error+=r.SetData(7,37);        //flags

    r.SetFilledFlag(true);          //Use this to write the data strings??

    r.SetNode(GSINodeThis);               //node attached to pc, (in this instance it is node 0, a CAN master)

    error+=r.SetType(rtGSIR2RxCANMessageNode2PC);  //This is an implicit call to FillCANFrame, as long as it finds a rtGSIR2RxCANMessageNode2PC
    //r.FillGSICANFrame();                  //can also FillCANFrame() if we have modified anything else
    wxASSERT(error ==0);

        wxString s;
     // \ is continuation character
    s.Printf(_("GSISocketAsyncRecord:\n"
    "-----------------------------------------\n"
    "Type=%s\nNode=%s\nSubfunction=%s\nData=%s\n"
    "------------------------------------------\n"),
    r.GetTypeString(),
    r.GetNodeString(),
    r.GetSubfuncString(),
    r.GetDataString()                    );

    TextCtrl()->AppendText(s);
    if(r.GetType()==rtGSIR2RxCANMessageNode2PC)
    {
        GSICANFrame &frame=r.GetGSICANFrame();
        s.Printf(_("CAN frame received:\n"
        "-----------------------------------------\n"
        "CAN id=%s\n"
        "Origin node=%s\n"
        "subfunction=%s\n"
        "Data=%s\n"
        "%s"                     //a blank string or InformationString1
        "%s"                     //a blank string or InformationString2
        "-----------------------------------------\n"
        ),
        frame.GetIDString(),
        frame.GetOriginNodeString(),
        frame.GetSubfuncString(),
        frame.GetDataString(),
        frame.GetInformationString1(),
        frame.GetInformationString2()
        );
        TextCtrl()->AppendText(s);

    }
#endif

#if 0
//write an arbitrary CAN message
                CANFRAME   f;
                f.id=GSICANIDLCDMessage;// +GSICANFrameBase;            //send an LCD message to node 1 (unshifted for node)
                f.buffer[0]=GSILCD::WriteString;    //subfuction
                f.buffer[1]='G';
                f.buffer[2]='l';
                f.buffer[3]='e';
                f.buffer[4]='n';
                f.buffer[5]='n';
                f.length=6;
                f.flags=CANXTDFrameNoRTR;  //
                r.FrameToRecord(&f);
                r.SetNode(1);
                //rv=Node()->Device()->Write(r);

//Ping a node
#if 0
                GSIRecord r1(rtGSICANPingMessage);
                r1.SetData(0,1);        //ping node 1
                r1.SetLength(1);
                rv=Node()->Device()->Write(r1);
#endif

#endif
}




//----------------------------------------------------------------------------------------------------------

GSITestPanel::GSITestPanel( wxFrame *frame, int x, int y, int w, int h )
       : wxPanel( frame, wxID_ANY, wxPoint(x, y), wxSize(w, h) )
//----------------------------------------------------------------------------------------------------------
{
    m_text = DEBUG_NEW wxTextCtrl(this, wxID_ANY, _T("This is the log window.\n"),
                            wxPoint(0, 250), wxSize(100, 50), wxTE_MULTILINE);
    m_text->SetBackgroundColour(wxT("wheat"));

    m_logTargetOld = wxLog::SetActiveTarget(new wxLogTextCtrl(m_text));

    m_book = DEBUG_NEW wxBookCtrl(this, wxID_ANY);



    wxPanel *panel = DEBUG_NEW wxPanel(m_book);

    m_text1 = DEBUG_NEW wxTextCtrl(this, wxID_ANY, _T("This is the log window.\n"),
                            wxPoint(0, 250), wxSize(100, 50), wxTE_MULTILINE);

    m_book->AddPage(panel,_("Test"),true);

    // sizer



}


GSITestPanel::~GSITestPanel()
{
    //wxLog::RemoveTraceMask(_T("focus"));
    delete wxLog::SetActiveTarget(m_logTargetOld);


}



#endif  //DIALOG_BLOCKS


