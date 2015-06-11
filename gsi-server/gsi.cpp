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
#  pragma implementation "gsi.cpp"
#  pragma interface "gsi.cpp"
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
#include "include/device.hpp"
#include "include/node.hpp"
#include "include/rs232.hpp"
#include "include/lcd.hpp"
#include "include/GSICq.hpp"
#include "include/GSISocketCommands.hpp"

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

// --------------------------------------------------------------------------
// resources
// --------------------------------------------------------------------------

// the application icon
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "mondrian.xpm"
#endif


// --------------------------------------------------------------------------
// constants
// --------------------------------------------------------------------------

//int fred=wxEVT_FIRST;

enum
{
  // menu items
  SERVER_QUIT = wxID_EXIT,
  SERVER_ABOUT = wxID_ABOUT,

  // id for sockets
	DEVICE_OPEN_RS232=100,
	DEVICE_OPEN_CAN,


    GSI_ID_USE_STRING,
    GSI_ID_COM_SCAN,
    GSI_ID_CAN_SCAN,

    GSI_ID_NULL,

	GSI_CUSTOM,


};


// --------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// --------------------------------------------------------------------------

DEFINE_EVENT_TYPE(wxEVT_GSI_ASYNC_RECORD)
DEFINE_EVENT_TYPE(wxEVT_GSI_MC_BREAK)
//DEFINE_EVENT_TYPE(wxEVT_GSI_SOCKET_COMMAND)   //now defined in socket.hpp so that client can use the same definition

BEGIN_EVENT_TABLE(GSIFrame, wxFrame)
    EVT_MENU(SERVER_QUIT,  GSIFrame::OnQuit)
    EVT_MENU(SERVER_ABOUT, GSIFrame::OnAbout)
	EVT_MENU(DEVICE_OPEN_RS232, GSIFrame::OnOpenDeviceRS232)
	EVT_MENU(DEVICE_OPEN_RS232, GSIFrame::OnOpenDeviceCAN)

    EVT_MENU(GSI_ID_COM1,GSIFrame::OnOpenNode)
    EVT_MENU(GSI_ID_COM2,GSIFrame::OnOpenNode)
    EVT_MENU(GSI_ID_COM3,GSIFrame::OnOpenNode)
    EVT_MENU(GSI_ID_COM4,GSIFrame::OnOpenNode)
    EVT_MENU(GSI_ID_COM5,GSIFrame::OnOpenNode)
    EVT_MENU(GSI_ID_COM6,GSIFrame::OnOpenNode)
    EVT_MENU(GSI_ID_COM_SCAN,GSIFrame::OnNodeCOMScan)
    EVT_MENU(GSI_ID_CAN_SCAN,GSIFrame::OnNodeCANScan)

	EVT_MENU(GSI_CUSTOM, GSIFrame::OnFireCustom)

	EVT_GSI_ASYNC_RECORD(wxID_ANY, GSIFrame::OnProcessAsyncRecord)
    EVT_GSI_MC_BREAK(wxID_ANY, GSIFrame::OnGSIMCBreak)                //microcontroller has signalled a BREAK (rs232)
    EVT_CLOSE(GSIFrame::OnCloseWindow)
END_EVENT_TABLE()


// --------------------------------------------------------------------------
// main frame
// --------------------------------------------------------------------------

/****************************************************************************
		ctor
GSIFrame::GSIFrame()
****************************************************************************/

GSIFrame::GSIFrame(const GSIServer *sock) : wxFrame((wxFrame *)NULL, wxID_ANY,
                             _("wxSocket demo: Server"),
                             wxDefaultPosition, wxSize(400, 200))
{
int i;
ErrorClass err(errNone);
wxString s;

    //memoryLeak=DEBUG_NEW char[100];
    try
    {
		m_SockServer=NULL;
		m_Text=NULL;
		m_CurrentDevice=NULL;
        // Give the frame an icon
        SetIcon(wxICON(MONDRIAN));                  //SetIcon is member of wxFrame?? or at least some base class
        // Make menus
        m_menuFile = DEBUG_NEW wxMenu();
        //m_menuFile->Append(SERVER_ABOUT, _("&About...\tCtrl-A"), _("Show about dialog"));
        //m_menuFile->AppendSeparator();
        m_menuFile->Append(SERVER_QUIT, _("E&xit\tAlt-X"), _("Quit server"));


        m_menuDevice = DEBUG_NEW wxMenu();
	    m_menuDevice->Append(DEVICE_OPEN_RS232, _("Open &RS232 communication to microcontroller"), _("Connect to a microcontroller using RS232"));


        m_menuDevice->Append(DEVICE_OPEN_CAN, _("Open &CAN communication to microcontroller"), _("Connect to a microcontroller using native CAN"));
	    m_menuDevice->Enable(DEVICE_OPEN_CAN,false);	//not available as yet
      //m_menuDevice->AppendSeparator();

        m_menuNode = DEBUG_NEW wxMenu();
	    m_menuNode->Append(GSI_ID_COM1, _("Open node on COM&1"), _("Connect to a microcontroller using COM1"));
	    m_menuNode->Append(GSI_ID_COM2, _("Open Node on COM&2"), _("Connect to a microcontroller using COM2"));
	    m_menuNode->Append(GSI_ID_COM3, _("Open Node on COM&3"), _("Connect to a microcontroller using COM3"));
	    m_menuNode->Append(GSI_ID_COM4, _("Open Node on COM&4"), _("Connect to a microcontroller using COM4"));
	    m_menuNode->Append(GSI_ID_COM5, _("Open Node on COM&5"), _("Connect to a microcontroller using COM5"));
	    m_menuNode->Append(GSI_ID_COM6, _("Open Node on COM&6"), _("Connect to a microcontroller using COM6"));
        m_menuNode->AppendSeparator();
        m_menuNode->Append(GSI_ID_COM_SCAN,  _("&Scan for nodes on COM ports"), _("Connect to a microcontroller(s) using COM port"));
        m_menuNode->Append(GSI_ID_CAN_SCAN,  _("&Scan for CAN nodes"), _("Search for CAN nodes"));

	    m_menuDebug = DEBUG_NEW wxMenu();
        m_menuDebug->Append(GSI_CUSTOM, _T("Fire GSI c&ustom event\tCtrl-U"),
                          _T("Generate a GSI custom event"));


        m_menuHelp = DEBUG_NEW wxMenu();
        m_menuHelp->Append(SERVER_ABOUT, _("&About...\tCtrl-A"), _("Show about dialog"));


      // Append menus to the menubar
        m_menuBar = DEBUG_NEW wxMenuBar();
        m_menuBar->Append(m_menuFile, _("&File"));
	    m_menuBar->Append(m_menuDevice, _("&Device"));
        m_menuBar->Append(m_menuNode, _("&Node"));
	    m_menuBar->Append(m_menuDebug, _("D&ebug"));
	    m_menuBar->Append(m_menuHelp, _("&Help"));
        SetMenuBar(m_menuBar);

    #if wxUSE_STATUSBAR
      // Status bar
        m_StatusBar=CreateStatusBar(GSIServerNumStatusBarFields);
    #endif // wxUSE_STATUSBAR

      // Make a textctrl for logging
        m_Text  = DEBUG_NEW wxTextCtrl(this, wxID_ANY,
                               _("Welcome to wxSocket demo: Server\n"),
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY);
        for(i=0;i<GSIMaxNumNodes;i++)
            m_Node[i]=NULL;
        for(i=0;i<GSIMaxNumDevices;i++)
        {
#if GSI_DUPLICATE_WITH_CONTAINERS
#error Don't use this #define yet, there are problems with it
            m_deviceHandle_Vec.push_back(GSINotAValidDevHandle);
#endif
            m_deviceHandle[i]=GSINotAValidDevHandle;

        }
        m_CurrentDevice=NULL;
        m_rs232DeviceOpen=false;             //debugging only
        //m_NodeThis=NULL;
#ifdef __VISUALC__
#warning ("2008 mod, remove next line")
#endif
        //wxCondition cond(wxMutex mut);

    //if(sock==NULL)  //any other value means the caller will instantiate the server
    if(sock==NULL)  //any other value means the caller will instantiate the server
        SetSockServer(DEBUG_NEW GSIServer(GSISocketAdress,StatusBar(),TextCtrl(),this));

	    UpdateStatusBar();
    }
    catch(gsException &exc)
    {
        wxString s;
        //s.Printf("Exception: %s. Error code=%i",exc.GetUserString(),exc.GetErrorCode());
		s.Printf("Exception: %s. \nError code=%i %s",exc.GetUserString().c_str(),exc.GetErrorCode(),exc.GetString().c_str());
        TextCtrl()->AppendText(s);
        wxMessageBox(_(s),
               _("Error"),
               wxOK | wxICON_EXCLAMATION, this);
    }
}

/************************************
    dtor
GSIFrame::~GSIFrame()
************************************/
GSIFrame::~GSIFrame()
{
  // No delayed deletion here, as the frame is dying anyway
  //delete m_server;
	if(m_SockServer)
		delete m_SockServer;
    for(GSINodeHandle h=0;h<GSIMaxNumNodes;h++)
        RemoveNode(h);
    if(m_CurrentDevice)
        delete m_CurrentDevice;

}

// event handlers
/*********************************************************
void GSIFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
*********************************************************/
void GSIFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
  // true is to force the frame to close

    //m_Node=NULL;
    Close(true);
}

/**********************************************************
void GSIFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
**********************************************************/
void GSIFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxMessageBox(_("GSI pic microcontroller interface. Written by Glenn Self 2005-6 \n"),
               _("About GSI"),
               wxOK | wxICON_INFORMATION, this);
}

/***************************************************
void GSIFrame::Test1(wxSocketBase *sock)
***************************************************/
void GSIFrame::Test1(wxSocketBase *sock)
{
  unsigned char len;
  char *buf;

  TextCtrl()->AppendText(_("Test 1 begins\n"));

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
  TextCtrl()->AppendText(_("Got the data, sending it back\n"));

  // Write it back
  sock->Write(buf, len);
  delete[] buf;

  TextCtrl()->AppendText(_("Test 1 ends\n\n"));
}

void GSIFrame::Test2(wxSocketBase *sock)
{
#define MAX_MSG_SIZE 10000

  wxString s;
  wxChar *buf = DEBUG_NEW wxChar[MAX_MSG_SIZE];
  wxUint32 len;

  TextCtrl()->AppendText(_("Test 2 begins\n"));

  // We don't need to set flags because ReadMsg and WriteMsg
  // are not affected by them anyway.

  // Read the message
  len = sock->ReadMsg(buf, MAX_MSG_SIZE * sizeof(wxChar)).LastCount();
  s.Printf(_("Client says: %s\n"), buf);
  TextCtrl()->AppendText(s);
  TextCtrl()->AppendText(_("Sending the data back\n"));

  // Write it back
  sock->WriteMsg(buf, len);
  delete[] buf;

  TextCtrl()->AppendText(_("Test 2 ends\n\n"));

#undef MAX_MSG_SIZE
}

void GSIFrame::Test3(wxSocketBase *sock)
{
  unsigned char len;
  char *buf;

  TextCtrl()->AppendText(_("Test 3 begins\n"));

  // This test is similar to the first one, but the len is
  // expressed in kbytes - this tests large data transfers.

  sock->SetFlags(wxSOCKET_WAITALL);

  // Read the size
  sock->Read(&len, 1);
  buf = DEBUG_NEW char[len * 1024];

  // Read the data
  sock->Read(buf, len * 1024);
  TextCtrl()->AppendText(_("Got the data, sending it back\n"));

  // Write it back
  sock->Write(buf, len * 1024);
  delete[] buf;

  TextCtrl()->AppendText(_("Test 3 ends\n\n"));
}


/*************************************************************
void GSIFrame::Test4(wxSocketBase *sock)

Received a record from the client
*************************************************************/
void GSIFrame::Test4(wxSocketBase *sock)
{
#define MAX_MSG_SIZE 10000

    wxString s;
    GSIChar *buff = DEBUG_NEW GSIChar[MAX_MSG_SIZE];
    wxUint32 len;

    TextCtrl()->AppendText(_("GSI send record\n"));

    GSINode * n;
	n=Node();

	if(Node()->Device()->GetMode() != GSI_MODE_BINARY)
		{
			Node()->Device()->SetMode(GSI_MODE_BINARY);
		}
  // We don't need to set flags because ReadMsg and WriteMsg
  // are not affected by them anyway.

  // Read the message
  len = sock->ReadMsg(buff, MAX_MSG_SIZE * sizeof(wxChar)).LastCount();

	GSIRecord r(buff,len);

  s.Printf(_("Client says: %s\n"), buff);
  TextCtrl()->AppendText(s);
  TextCtrl()->AppendText(_("Sending the data back\n"));

  // Write it back
  sock->WriteMsg(buff, len);
  delete[] buff;

  TextCtrl()->AppendText(_("GSI test ends\n\n"));

	//m_node->Device->Write(r);

#undef MAX_MSG_SIZE
}

// convenience functions

/*************************************************************************
		void GSIFrame::UpdateStatusBar()
**************************************************************************/
void GSIFrame::UpdateStatusBar()
{
#if wxUSE_STATUSBAR
  wxString s;
//GSI_STATUS_BAR_Frame=0,			//general status
//GSI_STATUS_BAR_Node,					//node status
//GSI_STATUS_BAR_Device,				//Device status
//GSI_STATUS_BAR_SOCKET		//socket status
	try	//accessing a Node() that is NULL with GSI_THROW, not fatal, so display fact on status line
	{
		if(Node())			//Make sure we have a node instantiated
		{
			s.Printf(_("Node:%d"),Node()->GetNodeHandle()); //will be 0-512
			SetStatusText(s, GSI_STATUS_BAR_NODE);
			s="Ascii";
			if (Node()->Device()->GetMode()== GSI_MODE_BINARY)
				s="Binary";
			SetStatusText(s, GSI_STATUS_BAR_DEVICE);
		}
	}
	catch (gsException &exc)
	{
		exc;
		s.Printf(_("No Node:")); //will be 0-512
		SetStatusText(s, GSI_STATUS_BAR_NODE);
		s="No Device";
		SetStatusText(s, GSI_STATUS_BAR_DEVICE);
	}

	//Server

	try
	{
		GetGSIServer();	//will exception if no server allocated
		{
			s.Printf(_("Clients:%d"), GetGSIServer().NumClients());
			SetStatusText(s, GSI_STATUS_BAR_SOCKET);
			if(GetGSIServer().NumClients())
			{
				SetIcon(wxICON(mondrian));
			}
			else
			{
				SetIcon(wxICON(connect));
		    }
		}
	}
	catch (gsException &exc)
	{
		exc;
		SetIcon(wxICON(mondrian));
	}
#endif // wxUSE_STATUSBAR
}

/***************************************************************************
    GSIFrame::OnOpenDeviceRS232
Look for a node on this device.
If one exists:
 # Instantiate a GSINode, set its Device to be the RS232Port

 # Place its address in the m_NodeThis field so that we can use the node without
   its hardware node address as this will only be valid once a CAN is running.

 # Request its node address (0-249). A value of 255 indicates there is no CAN
   currently running. If a CAN is running a value between 0-249 will be returned
   and is the node address. The GSINode* will be placed in  m_NodeArray[node address]

If no node on this port, throw an exception and exit.
If the OpenDevice fails, throw an exception and exit.

To have 2 nodes attached to the pc through seperate RS232 ports requires that
a CAN is running. This is because there is only a single m_NodeThis allowing
only one default node.

Search for valid nodes through the default m_NodeThis to find the node address
of other nodes on the network and instantiate them. Each node in the array can
use its own Device attached to the same pc, or can communicate with the pc
to and from the default node.

***************************************************************************/
void GSIFrame::OnOpenDeviceRS232(wxCommandEvent& event)
{
errorType rv=errNone;
GSIHandle handle;
GSIHandle node;

    event;
    wxString dev;
    dev=GetDeviceString(GSI_ID_COM2);       //need to extract this from event
    rv=OpenDevice(dev, &handle,&node);
}

/**************************************************
errorType GSIFrame::OnOpenDeviceCAN
Not implemented
***************************************************/
void GSIFrame::OnOpenDeviceCAN(wxCommandEvent& event)
{
    event;
}


void	GSIFrame::SetSockServer(GSIServer * s)
{
	m_SockServer=s;
}


/*!
errorType GSIFrame::CloseDevice(GSIHandle dhandle)

Close a device and its associated open nodes etc.
*/
errorType GSIFrame::CloseDevice(GSIHandle dhandle)
{
	if(dhandle==GSINotAValidDevHandle)
		return errBadHandle;
#if GSI_DUPLICATE_WITH_CONTAINERS
    if(m_deviceHandle_Vec.at(dhandle) == GSINotAValidDevHandle)     //can throw
        return errBadHandle;
    CloseAllNodesOnDevice(m_deviceHandle_Vec.at(dhandle));     //will set to NULL any LCD/Node device pointers
    //Although this syntax supports multiple devices, there is currently only one device allowed. More might be added later
    //delete m_CurrentDevice;
    m_deviceHandle_Vec.at(dhandle)=GSINotAValidDevHandle;
    delete m_CurrentDevice;
    m_CurrentDevice=NULL;
    for(GSIHandle i=0; i<GSIMaxNumDevices;i++)
    {
        //if(m_deviceHandle[i] != GSINotAValidDevHandle)
        //    wxFAIL_MSG("incorrect device handle was sent");
        m_deviceHandle_Vec.at(i)=GSINotAValidDevHandle;

    }
//Vector version above
#endif

    if(m_deviceHandle[dhandle] == GSINotAValidDevHandle)
        return errBadHandle;
    CloseAllNodesOnDevice(m_deviceHandle[dhandle]);     //will set to NULL any LCD/Node device pointers
    //Although this syntax supports multiple devices, there is currently only one device allowed. More might be added later
    //delete m_CurrentDevice;
    m_deviceHandle[dhandle]=GSINotAValidDevHandle;
    delete m_CurrentDevice;
    m_CurrentDevice=NULL;
    for(GSIHandle i=0; i<GSIMaxNumDevices;i++)
    {
        //if(m_deviceHandle[i] != GSINotAValidDevHandle)
        //    wxFAIL_MSG("incorrect device handle was sent");
        m_deviceHandle[i]=GSINotAValidDevHandle;
    }

    return errNone;
}



errorType GSIFrame::OpenDevice(const wxString &dev_str, GSIHandle *dev_handle)
{
bool isOpen;

    *dev_handle=GetDeviceHandle(dev_str,&isOpen);
	if(isOpen)
		return errAlreadyOpen;
/*
Max baud rate is 115200. Though the sdk lists others, the max is
115200.
pic 18f458 can generate close enough with a 20MHz crystal. It is not
close enough with a 10MHz crystal with or without HSPLL (internal 40MHz)
The HSPLL mode still uses the underlying 10MHz crystal for baud rate
generation.

I have verified this with the cro. The following lines (gsBAUD_256000) generate 8.8uS
square waves, approx 114kHz. As does gsBAUD_115200
*/

    GSIRS232Device *rsDev=DEBUG_NEW GSIRS232Device(dev_str,gsBAUD_115200,1024,*dev_handle,this); //instantiate the RS232Device
    if(rsDev->Open(dev_str,gsBAUD_115200,1024))
    {
        delete rsDev;
        return errFileOpen;             //probably already in use
    }
#if GSI_DUPLICATE_WITH_CONTAINERS
    m_deviceHandle_Vec.at(*dev_handle)=*dev_handle;
#endif
    m_deviceHandle[*dev_handle]=*dev_handle;

    m_CurrentDevice=rsDev;

    return errNone;
}



//------------------- GSIFrame::OpenDevice(int id) ----------------------------
errorType GSIFrame::OpenDevice(const wxString &dev_str, GSIDevHandle *dev_handle,GSINodeHandle *node_handle)
{
#ifdef __VISUALC__
#warning ("Convert str to id, check m_deviceHandle to see if open")
#endif
bool isOpen;


    *dev_handle=GetDeviceHandle(dev_str,&isOpen);
    if(isOpen)    //already open so user has selected 'close'
	{
        return errAlreadyOpen;
#if 0
	    Node()->LCD()->Cls();
		Node()->LCD()->Write("Bye");
		errorType rv=Node()->Device()->SetMode(GSIModeAscii);
        if(rv)
            TextCtrl()->AppendText(_("Node: failed to set ascii mode.\n"));
        else
            TextCtrl()->AppendText(_("Node: set ascii mode.\n"));
		UpdateStatusBar();
        Node()->Device()->Close();
		RemoveNode();       //defaults to GSI_NODE_THIS

		m_menuDevice->SetLabel(DEVICE_OPEN_RS232, _("Open &RS232 communication to microcontroller"));
		m_rs232DeviceOpen=false;
        m_deviceHandle[*dev_handle]=GSINotAValidDevHandle;
		TextCtrl()->AppendText(_("Closing RS232 device.\n"));
#endif
	}
	else
	{
		try
		{
	    	errorType rv;
            //OpenNode(GSI_NODE_THIS);
            CreateNode(*dev_handle);			//changed 21/02/08
/************************************************************************************
This is a request for an rs232 node, so instantiate a GSIRS232Device
GSINode m_Node		:Allows Device to call Node members
wxString &device	: The device name "com1", "com2" etc
CBR_115200				:baud_rate
size_t q_size			:size of circular queues used in object
GSIFrame * frame	:this top level window. Allows object to pass up socket events.
************************************************************************************/
            GSIRS232Device *rsDev=DEBUG_NEW GSIRS232Device(dev_str,gsBAUD_115200,1024,*dev_handle,this); //instantiate the RS232Device
            if(rsDev->Open(dev_str,gsBAUD_115200,1024))
                return errFileOpen;             //probably already in use
#if GSI_DUPLICATE_WITH_CONTAINERS
            m_deviceHandle_Vec.at(*dev_handle)=*dev_handle;
#endif
            m_deviceHandle[*dev_handle]=*dev_handle;

            Node()->SetNodeDevice(rsDev);
            Node()->Device()->SetDevice(rsDev);                                                  //grant access through Node
            //rsDev->InitComPort("COM2",gsBAUD_115200,1024);                  //Instantiate the low level serialPort, add to device

            //Node()->Device()->GSIDevice::SetDevice(this);


			Node()->Device()->SetCustomEventHandler(this);
			//m_Node=GSIRS232Node(wxString device,long baud_rate,size_t qsize,wxWindow *parent,wxEvtHandler *sock_event_handler)

			//(GSIChar node,GSIDevice *device,wxWindow *parent=NULL,wxEvtHandler * sock_event_handler=NULL); //for remote nodes.
			m_menuDevice->SetLabel(DEVICE_OPEN_RS232, _("Close &RS232 communication to microcontroller"));
			m_rs232DeviceOpen=true;  //this opens the com port. Even if the device is powered off it is still open
                            //so don't set to close
			TextCtrl()->AppendText(_("Open RS232 device (COM2,115200,1,N,8).\n"));
			rv=Node()->Device()->SetMode(GSI_MODE_BINARY);
			if(rv)
            {
                TextCtrl()->AppendText(_("(Failed to find a node).\n"));
				return rv;
            }
            /*Need to find how many nodes on the CAN.
            If this node is the master, ask it to enumerate how many nodes it is in contact with
            If this node is a slave, ask the master for its node list

            All nodes should send a message to any attached rs232 port when a node is assigned to them
            Must also be able to read the currently attached node id.
            Must also be able to read all CAN Nodes on the system
            */

   			GSINode *n;
			n=Node();

			Node()->LCD()->Cls();                   //defaults to GSI_NODE_THIS
			Node()->LCD()->Write("Hello Glenn!");
            *node_handle=Node()->GetNodeHandle();

            return errNone;
#if 0
            int hdwNode;

            rv=Node()->ReadNode(&hdwNode);     //ReadNode obtains the node id from the hardware attached to this node

            if(rv==errNone && hdwNode < GSIMaxNumPhysicalNodes)
            {
                int i;
                for(i=0;i<GSIMaxNumPhysicalNodes;i++)
                {
                    //ping all the nodes. Should receive async ping records back for all that exist
                    GSIRecord r1(rtGSICANPingMessage);
                    r1.SetData(0,(GSIChar)i);        //ping node 1
                    r1.SetLength(1);
                    rv=Node()->Device()->Write(r1);
                }
            }
#endif
//#warning ("Can use data as integer quantities, set to 0xffff, then when we touch the array, we can count data items")
//above isn't a complete solution. Better for the SetTypeString to fill the length with reasonable defaults or to
//stay as we are and keep length as zero by default to minimise damage.
        }
		catch(gsException &exc)
		{
			wxString s;
			s.Printf( "%s\nFailed to open RS232 device.\n\n",exc.GetString().c_str());
			TextCtrl()->AppendText(s);
            return exc.GetErrorCode();
		}
	}

    UpdateStatusBar();
    return errNone;
}


// ----------------------------------------------------------------------------
// custom event methods
// ----------------------------------------------------------------------------

//void GSIFrame::OnFireCustom(wxCommandEvent& WXUNUSED(event))
void GSIFrame::OnFireCustom(wxCommandEvent& event)
{
    event;
	wxCommandEvent eventCustom(wxEVT_GSI_MC_BREAK);
#if 0
	GSIRecord * r=DEBUG_NEW GSIRecord(rtGSILCDMessage);
	r->SetLength(2);
	r->SetData(0,0x31);
	r->SetData(1,0x32);
	eventCustom.SetClientObject((wxClientData *)r);
#endif
    wxPostEvent(this, eventCustom);
}

/**********************************************************************************
void GSIFrame::OnProcessAsyncRecord(wxCommandEvent& event)

GSIDevice (GSISerialThread if using GSIRS232Device) has detected
a record sent from the microcontroller.
The record is in the AsyncQ

We will pass it through the socket as a string. First byte will be an id
must be sure that the ID's don't conflict with wxIDs as the other end of the socket
will be receiving socket events

This function can receive data from multiple nodes/devices. It is not sufficient
to use the default Node()->functions()
The event probably needs to have the node embedded in it
Node is embedded in wxCommandEvent using SetInt()
***********************************************************************************/
//void GSIFrame::OnProcessAsyncRecord(wxCommandEvent& WXUNUSED(event))

void GSIFrame::OnProcessAsyncRecord(wxCommandEvent& event)
{
static int inFunction=0;
    inFunction++;
    wxASSERT_MSG(inFunction < 2,"OnProcessAsyncRecord called recursively" );
    //GSIRecord *r=(GSIRecord *) event.GetClientObject();
    GSIRecord r;
    GSINodeHandle n;
    GSINode * tNode;

    n=(GSINodeHandle) event.GetInt();               //the node will be set in here by the poster
    wxASSERT (n <= GSIMaxNumNodes);
    //tNode=Node(n);
                    //find the correct node
    tNode=Node(n);
    tNode->Device()->AsyncQ()->Read(r);
    TextCtrl()->AppendText(_("Sending Async GSIRecord\n"));

#if ADD_ASYNC_NODE_IN_GSIFRAME //added 12/4/12
    if(r.GetType()==rtGSIR2RxCANMessageNode2PC)
    {

	    //A can message
	    GSICANFrame rxFrame;
	    errorType rv=r.MakeGSICANFrame(rxFrame);
	    if(rv)
	    {
		    wxLogMessage("%i %s Record2Frame error\n\r",__LINE__,__FILE__);
		    return;
	    }
//int n=rxFrame.GetOriginNode();
//Frame()->
//note the id also contains the CANNode of the receiver (attached to this device) as base 4096
//ie CANNode 0=0, CANNode 1=4096, CANNode 2= 8192
//if CANID was 1 then 4097 would be an ID of 1 can frame
//to CANNode 1, caught here, therefore our node

        //A node can send us a message before we know it is online, before we have allocated a Node for it
        GSIChar n=rxFrame.FindOriginNode();

        if(Node(n) == NULL)        //Don't have a node object for this hardware
        {
            //Frame()->AddNode(n);            //so create one
            CreateNode(tNode->GetDeviceHandle(),n);            //so create one
        }
		wxASSERT_MSG(Node(n) != NULL,"No node allocated");
        Node(n)->DefaultProcessAsyncCANFrame(rxFrame);
    }

#else
    tNode->DefaultProcessAsyncRecord(r);      //despatch to correct node object
#endif

    if (GetGSIServer().NumClients())
    {
	    //unsigned char c=GSI_SOCKET_ASYNC_RECORD;
	    //SockServer()->GSIClient()->Write(&c,1);	//tell client it is a TestRecord
	    //convert record to a wxString
	    //wxString s;
        GSIChar length;
	    r.SetFilledFlag(true);

	    //r.RecordToWxString(s);                      //convert record to str
        GSIChar buff[GSIRecordMaxLength+2];
        length=r.RecordToBuffer(buff);
        //wxString(const char* psz, size_t nLength = wxSTRING_MAXLEN)
        wxString s(buff, length);


        s=(char)GSI_SOCKET_ASYNC_RECORD+s;            //prepend the id
        length++;
	    //int l=s.Length();
	    wxASSERT_MSG(length <GSIRecordMaxLength+2,"Socket data too large");//+1 for zero index, +1 for length byte

	    //SockServer()->GSIClient()->WriteMsg(s,length);
        wxSocketBase& sock=tNode->GetSocket();
        sock.WriteMsg(s,length);
	    TextCtrl()->AppendText(_("Sent Async GSIRecord\n"));
    }

    else
    {
	    TextCtrl()->AppendText(_("Failed, no socket open.\n"));
    }
    inFunction--;
}

/**********************************************************************************
void GSIFrame::OnGSIMCBreak(wxCommandEvent& event)
A device has received a BREAK  (an RS232 BREAK for example) with a type code
We must find the node attached to this device and remove it if it is a RESET
The mc will send a BREAK followed by a reset code
It will then look for an echo of the reset code. If it doesn't receive one
it will continue with its reset, returning to ascii mode etc.
If the mc is in ascii mode it will not send a reset

***********************************************************************************/

void GSIFrame::OnGSIMCBreak(wxCommandEvent& event)
{
wxString s,dev_name,info;
int resetType;
bool reset=true;

    GSIDevice *Dev=(GSIDevice *) event.GetClientData();
    resetType=event.GetInt();
    dev_name=Dev->GetDeviceName();

    switch (resetType)
    {
      case GSI_BREAK_RESET_WDT:              //Watch Dog Timer
        Dev->Write(GSI_BREAK_RESET_WDT);
        Dev->Write(GSI_ACK);
        info="Watch dog Timer";
        break;
    case GSI_BREAK_RESET_BOR:              //Brown Out
        Dev->Write(GSI_BREAK_RESET_BOR);
        Dev->Write(GSI_ACK);
        info="Brown out";
        break;
    case GSI_BREAK_RESET_UNKNOWN:          //pic chips (18F458 at least!) running under ICD2 generate this
        Dev->Write(GSI_BREAK_RESET_UNKNOWN);
        Dev->Write(GSI_ACK);
        info="Unknown";
        break;
    case GSI_BREAK_RESET_USER_ABORT:        //User abort, shouldn't see this in this version
        Dev->Write(GSI_BREAK_RESET_USER_ABORT);
        Dev->Write(GSI_ACK);
        info="User abort";
        break;
    case GSI_BREAK_RESET_POR:              //Power On
        Dev->Write(GSI_BREAK_RESET_POR);
        Dev->Write(GSI_ACK);
        info="Power on";
        break;
    case GSI_BREAK_RESET_STACK_OVF:              //Stack overflow
        Dev->Write(GSI_BREAK_RESET_STACK_OVF);
        Dev->Write(GSI_ACK);
        info="Stack overflow";
        break;
    case GSI_BREAK_RESET_STACK_UNF:              //Stack underflow
        Dev->Write(GSI_BREAK_RESET_STACK_UNF);
        Dev->Write(GSI_ACK);
        info="Stack underflow";
        break;
    case GSI_BREAK_RESET_FATAL:              //Fatal error either from mc or a comms failure, do not write back to mc
        //Dev->Write(GSI_BREAK_RESET_FATAL);
        //Dev->Write(GSI_ACK);
        info="Fatal";
        break;

    //best to treat SendBreak as a default case
    case GSI_BREAK_SEND_RECORD:            //mc wants to send us a record, it will become slave immediately after receiving our ACK
        reset=false;                    //not a reset
        //if we can't become slave then simply return. The mc will timeout and resume, trying again later
 //Master/Slave is largely meaningless. we have to use a mutex to determine who owns the Comm resource. Should probably remove Master
 //slave, though can still do an IsCommsMaster/IsCommsSlave by testing the mutex? Not really as when we are writing a record
 //the mx is locked, as it is when receiving an asyncronous one.
//        if(Dev->IsCommsMaster())
        {
            errorType rv;
            wxMutexLocker mx(Dev->GetCommsMutex());

            if(mx.IsOk() == false )
                break;                               //couldn't get lock, so ignore, pc must be doing something

//            if(Dev->SetSlave()!= errNone)            //make us the slave, this will Lock the CommsMutex, we *must* unlock it when finished!
//                return;                              //couldn't become slave, pc must be sending a record
//We are slave. We have the Comms mutex
            rv=Dev->Write(GSI_BREAK_SEND_RECORD);
            rv=Dev->Write(GSI_ACK);
            GSIRecord record;
            rv=Dev->Read(record,GSIRecordReadTimeout);      //probably should throw an exception on a timeout, mc will probably wdt reset in any case

			if(rv)
			{
		        wxMessageBox(_("Timeout on reading record (node will WDT?)"),
				_("Error"),
				wxOK | wxICON_EXCLAMATION, this);

				return;
			}
#if 0
            if(rv)
            {
                Dev->AsyncQ()->Flush();                     //clear out the Q in case we can recover
                break;
            }
			record.SetFilledFlag(true);
            CQueue *q=Dev->AsyncQ();
			rv=Dev->AsyncQ()->Write(record);                //place the record in the async Q
            if(rv)
                break;                                      //abort on error
#endif
            int node=Dev->Node()->GetNodeHandle();

            wxASSERT (node <= GSIMaxNumNodes);

            //rv=ProcessAsyncRecord(Node(node));
            rv=ProcessAsyncRecord(record,Node(node));
            //must now unlock the CommsMutex and become CommsMaster
            break;
//Should we unlock the mutex explicitly or allow SetMaster to do so?
        } //IsCommMaster
#if 0
        else
        {
            //already slave, looks like a problem
            wxFAIL_MSG("Tx slave request when already a slave");
        }
#endif
        break;

    case GSI_BREAK_SEND_MULTIPLE_RECORDS:   //mc wants to send more than one record. keep receiving till an RS232Mssg:ReqSlave is found
    case GSI_BREAK_SEND_DATA:
        wxFAIL_MSG("Unsupported");
        break;

    default:
    //can arrive here with reset_type of 0. POR will reset the USART. Might generate an extended BREAK condition
    //we read NULLS
        Dev->Write((GSIChar)resetType);
        Dev->Write(GSI_ACK);
        info="Unknown";
    }
    if(reset)
    {
        Dev->SetControllerHasReset(true);
        RemoveNode(Dev);
        s.Printf("Received %s RESET from node on %s\n",info.c_str(),dev_name.c_str());
        TextCtrl()->AppendText(s);
    }
}

/*------------------------------------------------------------------------------------
GSINode *const GSIFrame::Node(int n=GSI_NODE_THIS) const      //will default to GSI_NODE_THIS

Some housekeeping tasks wish to loop through all the nodes in NodeArray[]
by setting use_exception_flag to false they can do so without generating
exceptions
-------------------------------------------------------------------------------------*/

GSINode *const GSIFrame::Node(GSINodeHandle h,bool use_exception_flag) const      //will default to GSI_NODE_THIS
{
    //wxASSERT(m_Node[h]);
	if(m_Node[h] ==NULL)
	{
		if(use_exception_flag)
		{		//exception by default
			GSI_THROW("No node object (GSIFrame::Node)",errNoAlloc)
		}
	}
    return(m_Node[h]);          //there is an object here
}

/*********************************************************************
void GSIFrame::OnCloseWindow(wxCloseEvent& event)

close button 'X' pressed
***********************************************************************/
void GSIFrame::OnCloseWindow(wxCloseEvent& event)
{
#ifdef __VISUALC__
#warning ("Need to ensure clean up memory here!")
#endif
    event;
    this->Destroy();
}


/*------------------------------------------------------------------------*/
/*!
 *   errorType GSIFrame::RemoveNode(GSIHandle h_node)
 *
 * Remove a node from m_Node[]
 * deletes the GSINode object
 *returns
 *errNone      : success
 *errRange     : h_node out of range
 *errBadHandle : No node here, h_node is a bad handle
 *Must also see if the node is being used as GSI_NODE_THIS, NULL the
 *entry in m_Node[GSI_NODE_THIS] if it is as this is an alias
 *If asked to remove GSI_NODE_THIS
 *search the m_Node[] for its alia, delete if found then simply NULL m_Node[GSI_NODE_THIS] and return
 *if not found, then NULL m_Node[GSI_NODE_THIS]
*/
/*--------------------------------------------------------------------
            void GSIFrame::RemoveNode(int n)
---------------------------------------------------------------------*/
errorType GSIFrame::RemoveNode(GSIHandle h_node)
{
GSINode *node;

	if(h_node==GSI_NODE_THIS)
	{
		//ok, find the underlying node we are being asked to remove
		h_node=FindNodeHandle(m_Node[GSI_NODE_THIS]);
		m_Node[GSI_NODE_THIS]=NULL;		//delete the GSI_NODE_THIS if this is its alias (should always alias node 0 on a CAN server)

		if(h_node == GSINotAValidNodeHandle)
		{
			wxFAIL_MSG("Couldn't find parent GSINode *");
			return errNone;
		}
	}

    errorType rv=GetNode(h_node,&node);

    if(rv)
        return rv;

	if(node==m_Node[GSI_NODE_THIS])
		m_Node[GSI_NODE_THIS]=NULL;		//delete the GSI_NODE_THIS if this is its alias (should always alias node 0 on a CAN server)

    delete node;
	wxASSERT(m_Node[h_node]);			//quick check to make sure we have a node here!
    m_Node[h_node]=NULL;
	wxString s;
	s.Printf(_("Deleted node %i\n"),h_node);
	TextCtrl()->AppendText(s);

    return rv;
}

/*********************************************************************************
errorType   RemoveNode(GSIDevice *device)
**********************************************************************************/
errorType   GSIFrame::RemoveNode(GSIDevice *device)
{
bool found=false;
errorType rv=errNone;
int id,devType;
bool isOpen;

    found;
    if (device == NULL)
        return rv;


    id=GetDeviceHandle(device->GetDeviceName(),&devType,&isOpen);                         //returns ID so we can call Node->Open event
                                                                              //also return true/false if device is currently open

    wxCommandEvent evt;
    evt.SetId(id);
    OnOpenNode(evt);

#if 0
    if(m_Node[GSI_NODE_THIS]->Device()==device)     //unlink the default node if it is the node to be removed
        m_Node[GSI_NODE_THIS]=NULL;

    for(i=0;i<GSIMaxNumPhysicalNodes;i++)
    {
        if(m_Node[i]->Device()==device)
            delete m_Node[i];       //remove any nodes attached to this device
    }


    //If we now do not have a default node, look for one in the array. Use the first we find
    if(m_Node[GSI_NODE_THIS] == NULL)
    {
        for(int i =0;i<GSIMaxNumPhysicalNodes;i++)      //make default node a physical one if possible
        {
            if(m_Node[i])
            {
                m_Node[GSI_NODE_THIS]=m_Node[i];
                found=true;
                break;
            }
        }
    }

    if(found ==false)
    {
        for(i=GSIFirstVirtualNode;i<GSIMaxNumNodes;i++)    //no physical nodes, so look for first virtual node
        {
            if(m_Node[i])
            {
                m_Node[GSI_NODE_THIS]=m_Node[i];
                found=true;
                break;
            }
        }
    }
#endif
    return rv;
}

/*!
 *GSINodeHandle FindNodeHandle(const GSINode *n) const
 Given a GSINode *, find its corresponding handle
 NOTE
 Could also be in m_Node[GSI_NODE_THIS]
*/

GSINodeHandle GSIFrame::FindNodeHandle(const GSINode *n) const
{
bool found=false;
GSIHandle h;
	for(h=0;h<GSIMaxNumNodeHandles;h++)
	{
		if(h==GSI_NODE_THIS)
			continue;			//ignore GSI_NODE_THIS in this search
		if(m_Node[h]==n)
		{
			found=true;
			break;
		}
	}
	wxASSERT_MSG(!(found==false && m_Node[GSI_NODE_THIS]==n),"GSI_NODE_THIS has no object!");

	if(found)
		return h;
	return GSINotAValidNodeHandle;
}

/************************************************************************************
errorType FindNodeOnDevice::FindNodeOnDevice(wxString device,int *node,int *num_nodes)
*************************************************************************************/
errorType GSIFrame::FindNodeOnDevice(const wxString &device_name,GSIHandle *h_node,size_t *num_nodes) const
{

errorType rv=errFail;
GSIHandle i;
GSIDevice *dev;
    num_nodes=0;
    for(i=GSIMaxNumNodes;i!=0 ;i--)
    {
        dev=m_Node[i]->Device();
        if (dev->GetDeviceName() == device_name)
        {
            num_nodes++;
            *h_node=i;
        }
    }
    return(rv);
}

/*-----------------------------------------------------------------
    void GSIFrame::OnOpenNode(wxCommandEvent& event)
We are not limited to rs232 devices, so this  function needs
to instantiate a device of the correct type and send its open string
to the AddNode() function

Effectively this is a call to OpenNodeOnDevice()

Also called when we close a node, need to see if the node is open
or if the menu string is "Close" rather than "Open"

This is currently limited to opening and closing the default node
on a device. The client must be able to open multiple nodes on a single
device. This is for debugging purposes only at present.
To be useful, need to implement a dialog that obtains the node to open
on a particular device.
Would need a separate close menu

Wouldn't need the 6 separate open menu items

NOTE
If the device has been opened elsewhere, then we don't need to open it!
And IMPORTANT the logic if(isOpen) fails, because we are using it to
decide whether the menu reads Open/Close. The menu could read Open while
we are already open, this code would then Close, assuming it was a toggle!
------------------------------------------------------------------*/
void GSIFrame::OnOpenNode(wxCommandEvent& event)
{
errorType rv;
wxString devStr;
GSIHandle hnode;
GSIDevHandle hDevice;

bool isOpen=false;
//Find which device we are accessing
#ifdef __VISUALC__
#warning ("Code required to choose which node to open. Using GSI_NODE_THIS")
#endif
    hnode=GSI_NODE_THIS;
    devStr=GetDeviceString(event.GetId());

    hDevice=GetDeviceHandle(devStr,&isOpen);


    if(m_CurrentDevice && isOpen==true)
	{
		if(m_CurrentDevice->GetHandle() != hDevice) //if handles are == then close the device
		{
			wxMessageBox(_("Device already open (only one device allowed in this version)"),
			_("Error"),
			wxOK | wxICON_EXCLAMATION, this);
			return;
		}
	}
    if(isOpen)
    {//This is a menuCommand to close the node
        rv=RemoveNode(hnode);
        //if(rv == errNone)
        //XXXGS mod 7/4/12
        if(rv == errNone || rv==errNotOpen)
        {                       //succesfully removed node, so can now re-open it
            rv=CloseDevice(hDevice);
            if(rv)
                return;
            wxString s;
            s.Printf(_("Open node on %s"),devStr.c_str());
            int id=GetDeviceId(devStr);
            wxASSERT(id != -1);
            m_menuNode->SetLabel(id,s);

        }
        return;
    }
    //need to open the device
    rv=OpenDevice(devStr,&hDevice);
    if(rv)
    {
        ErrorClass err(rv);
        wxString s;
        s.Printf(_("Couldn't open %s.  %s"),devStr.c_str(),err.GetString().c_str());
        wxMessageBox(_(s),
       _("Error"),
       wxOK | wxICON_EXCLAMATION, this);
    }

    //need to open the node
    rv=CreateNode(hDevice,hnode,NULL);     //id will be an identifier for the client. We will send it back. Client will refer to this node through the handle
                                         //m_Node[] will store the GSINode* at m_Node[node]. This call sends a sock==NULL

//Probably need a separate close node menu as we might want to open another node on the same device
    if(rv==errNone)
    {                               //succesfully opened node, so must updata menu to allow closing node
        wxString s;
        int id=GetDeviceId(devStr);
        wxASSERT(id != -1);
        s.Printf(_("Close node on %s"),devStr.c_str());
        m_menuNode->SetLabel(id,s);

    }
}



/*!
 * errorType GSIFrame::OpenNode(int id)
 *Create a GSINode object attached to h_device
 *returns:
 *errNone on success
 *errNotOpen if h_device doesn't refer to an open GSIDevice or if the device cannot establish control with a microcontroller
 *errFail if GSINode cannot be instantiated
 *errAlreadyOpen if Node is already instantiated

 */

/*************************************************************************
errorType GSIFrame::OpenNode(int id)
note that if this is opened by the server directly there will be no socket
so sock==NULL
h_device should be an open device, if not:error (probably from the client)
***************************************************************************/
errorType GSIFrame::CreateNode(GSIHandle h_device, GSIHandle h_node,wxSocketBase *sock)
{
errorType rv=errNone;
GSINode *tNode=NULL;

    if(IsDeviceOpen(h_device) == false)
        return errNotOpen;
    try
    {
        tNode=DEBUG_NEW GSINode(this,GetDevice(h_device),h_node,sock);
    }
	catch(gsException &exc)
	{
		wxString s;
        ErrorClass e(exc.GetErrorCode());
        if (tNode)
            delete tNode;

		s.Printf( _("%s\nFailed to open node %i on device %s.\n\n"),exc.GetString().c_str(),h_node,GetDeviceString(h_device).c_str(),exc.GetUserString().c_str());
		TextCtrl()->AppendText(s);
        return errFail;
	}

    rv=AddNode(tNode,h_node);
	//At this point we have created a node object. It has succesfully set
	//binary mode, so a node exists.  It has also read the hardware node from
	//the node and they match. Return errNotExist otherwise
    if(rv)
    {
        delete tNode;
    }
    else
    {
		TextCtrl()->AppendText("Opened Node. In binary mode\n");
#if STARTLE_SERVER == FALSE
	    tNode->LCD()->Cls();
        tNode->LCD()->Write("Hello Glenn!");
#endif
    }
    return rv;
}


/*!
 * errorType GSIFrame::AddNode(GSINode *node,GSINodeHandle h_node)
 * Add a node to the m_Node[] with error checking
 * returns errFail if already occupied
 * return errRange if out of range
 * errNotOpen if couldn't open node
 *NOTE
 *If GSI_NODE_THIS node is requested, then open the node (if it exists), then
 *obtain the hardware node (GSINodeNoNetwork if no CAN running)
 *set m_Node[HardwareNode] and m_Node[GSI_NODE_THIS] with the GSINode *
 *GSI_NODE_THIS is a special entry, will only contain an alias and will not
 *be deleted, will simply be NULL'd
 *NOTE
 *GSINodeNoNetwork is another special case. If a network starts up, it
 *will change its hardware node. Code still to be written for this
*/
errorType GSIFrame::AddNode(GSINode *node,GSINodeHandle h_node)
{
errorType rv=errNone;
GSIHandle hardNode;

    if(h_node <0 || h_node >= GSIMaxNumNodes)   //see if a valid handle
        return errRange;

    if(m_Node[h_node])      //see if node slot is already occupied
        return errAlreadyOpen;

    node->Device()->SetCustomEventHandler(this);  //GSIFrame will handle events from device

    rv=node->Device()->SetMode(GSI_MODE_BINARY);
	if(rv)
    {
        TextCtrl()->AppendText(_("(Failed to find a node).\n"));
		return errNotOpen;
    }
	hardNode=node->GetHdwNode();
	if(h_node == GSI_NODE_THIS)	//open any node allowed
	{
		m_Node[GSI_NODE_THIS]=node;	//a copy, shouldn't be deleted
									//though must ne NULL'd when alias is deleted
	}
	else
	{
		if(hardNode != h_node)
		{
			return errNotExist;
		}
	}
    wxASSERT(m_Node[hardNode]==NULL);
	m_Node[hardNode]=node;
    return errNone;
}

/***********************************************************************
errorType GSIFrame::CloseNode(int id)
***********************************************************************/
errorType GSIFrame::CloseNode(int id)
{   id;

    return errFail;
}


/***************************************************************************************
errorType GSIFrame::SearchForNodes(GSIHandle handle,GSINetwork &net)
handle values are currently fixed to valid supported devices. Though we use an array
the array entries simply contain the index, though this may change later
***************************************************************************************/
errorType GSIFrame::SearchForNodes(GSIHandle handle,GSINetwork &net)
{
//The server should always be aware of the state of the network as nodes coming online should be
//found as they obtain or send a network id.
//need to search GSINodes in m_NodeArray[] for DeviceHandle. If found, then set net m_NodeArray to reflect

GSIHandle i;
GSINode *node;

    for(i=0;i<GSIMaxNumNodes;i++)
    {
        GetNode(i,&node);
        if(node->GetDeviceHandle()==handle)
            net.SetNode(i,node);
        else
            net.SetNode(i,NULL);
    }
    return errNone;
}


/*!
 *errorType   GSIFrame::GetNode(GSINodeHandle handle,GSINode **node) const
*/
errorType   GSIFrame::GetNode(GSINodeHandle handle,GSINode **node) const
{
    wxASSERT_MSG(handle >= 0 && handle <= GSIMaxNumNodes,"GetNode() handle out of range");

    if(handle < 0 || handle >= GSIMaxNumNodes)
        return errBadHandle;

    *node=m_Node[handle];

    if(*node == NULL)
        return errNotOpen;

    return(errNone);
}


/*-----------------------------------------------------------------
    void GSIFrame::OnNodeCOMScan(wxCommandEvent& event)
Scan all com ports (1-6) for a node and connect if available
Be sure to check that we are not already attached!
------------------------------------------------------------------*/
void GSIFrame::OnNodeCOMScan(wxCommandEvent& event)
{
wxString sDev,s;
//GSINodeHandle hNode;
GSIDevHandle dh;
GSIRS232Device *device=NULL;
errorType rv=errNone;

    event;
    for(dh=GSIFirstComPortHandle;dh<GSIFirstComPortHandle+GSIMaxNumComPorts;dh++)
    {
        //if(m_comPortOpen[i]==0)
#if GSI_DUPLICATE_WITH_CONTAINERS
        if(m_deviceHandle_Vec.at(dh)==GSINotAValidDevHandle)
        {
            try
            {
                sDev.Printf("COM%i",dh+1);
                device= DEBUG_NEW GSIRS232Device(sDev,gsBAUD_115200,1024,this);
                //ok we opened the device, now see if it is attached to a GSI node, will generate exception on failure
                //rv=AddNode(device, &hNode);
                rv=CreateNode(m_deviceHandle_Vec.at(dh));
                if(rv)      //Problem adding the node so delete the node and ignore the error
                {
//on falure, AddNode can either delete the device or leave it allocated. We must check it here
                    //if (device)
                    //    delete device;
                }
                else
                {
                    //m_comPortOpen[i]=node;     //a valid node has been returned so this is safe
                    m_deviceHandle_Vec.at(dh)=dh;     //a valid node has been returned so this is safe
                    s.Printf(_("Close node on COM&%dh"),dh+1);
                    m_menuNode->SetLabel(GSI_ID_COM1+dh,s);
                }
            }//try

             catch(gsException &exc)
            {
                exc;
                //can't open, already open etc. Ignore the error
                continue;           //continue the for loop
            }
        } //if m_comPortOpen

//End Vec version
#endif //#if GSI_DUPLICATE_WITH_CONTAINERS
        if(m_deviceHandle[dh]==GSINotAValidDevHandle)
        {
            try
            {
                sDev.Printf("COM%i",dh+1);
                device= DEBUG_NEW GSIRS232Device(sDev,gsBAUD_115200,1024,dh,this);
                //ok we opened the device, now see if it is attached to a GSI node, will generate exception on failure
                //rv=AddNode(device, &hNode);
                rv=CreateNode(m_deviceHandle[dh]);
                if(rv)      //Problem adding the node so delete the node and ignore the error
                {
//on falure, AddNode can either delete the device or leave it allocated. We must check it here
                    //if (device)
                    //    delete device;
                }
                else
                {
                    //m_comPortOpen[i]=node;     //a valid node has been returned so this is safe
                    m_deviceHandle[dh]=dh;     //a valid node has been returned so this is safe
                    s.Printf(_("Close node on COM&%dh"),dh+1);
                    m_menuNode->SetLabel(GSI_ID_COM1+dh,s);
                }
            }//try

             catch(gsException &exc)
            {
                exc;
                //can't open, already open etc. Ignore the error
                continue;           //continue the for loop
            }
        } //if m_comPortOpen

    }//for loop
}

/*-------------------------------------------------------------------------
    void GSIFrame::OnNodeCANScan(wxCommandEvent& event)

Scan for any nodes that might be on the network.
Check all hardware nodes. If any are found, use them to ping other nodes
on the CAN. This will result in all nodes that are attached pinging us back
which in turn will place then in the NodeArray
-------------------------------------------------------------------------*/

void GSIFrame::OnNodeCANScan(wxCommandEvent& event)
{
int i;
bool found=false;
wxString s;
errorType rv;
GSINode *node=NULL;
    event;
    for(i=0;i<GSIMaxNumPhysicalNodes;i++)
    {
        if(Node(i))
        {
            found=true;
            node=Node(i);
            break;      //found a node
        }
    }
    if(found)
    {
        for(i=0;i<GSIMaxNumPhysicalNodes;i++)
        {
            //ping all the nodes. Should receive async ping records back for all that exist
            GSIRecord r1(rtGSICANPingMessage);
            r1.SetData(0,(GSIChar)i);        //ping node i
            r1.SetLength(1);
            wxASSERT_MSG(node->Device(),"No Device attached to node!");

            rv=node->Device()->Write(r1);

#if 0
            rv=Node()->ReadNode(&hdwNode);     //ReadNode obtains the node id from the hardware attached to this node

            if(rv==errNone && hdwNode < GSIMaxNumPhysicalNodes)
            {
                int i;
                for(i=0;i<GSIMaxNumPhysicalNodes;i++)
                {
                    //ping all the nodes. Should receive async ping records back for all that exist
                    GSIRecord r1(rtGSICANPingMessage);
                    r1.SetData(0,(GSIChar)i);        //ping node 1
                    r1.SetLength(1);
                    rv=Node()->Device()->Write(r1);
                }
            }
#endif

            if(rv)
            {
                ErrorClass e(rv);
                s.Printf(_("Failed to write record (%s)"),e.GetString().c_str());
                wxMessageDialog dialog(NULL,
                            s,    //text
                            GSI_WARNING,           //caption
                            wxOK|wxICON_WARNING);
                dialog.ShowModal();
                return;
            }

        }
        s.Printf(_("Pinging nodes on (%s)"),node->Device()->GetDeviceName().c_str());
        wxProgressDialog pd(s ,                                           //title
                            s,                                              //message
                            100,                                            //maximum
                            this,                                           //parent
                            wxPD_APP_MODAL |wxPD_AUTO_HIDE );               //style
        wxStopWatch sw;
        i=200;
        do
        {
            if(sw.Time()>i)
            {
                pd.Update(i/20);
                i+=200;
            }
        }while (sw.Time()<1000); //pause for 1000 msec

    }
    //This is all we need to do, we don't know how long it will take for the nodes to respond
    //We should probably wait a timeout interval and then re-read the NodeArray to see how many
    //have been found
    //

}

/**************************************************************************************
errorType GSIFrame::GetDeviceHandle(const wxString &dev,GSIHandle *handle,bool *is_open)
default parameter is_open==NULL
***************************************************************************************/
//errorType GSIFrame::GetDeviceHandle(const wxString &dev,GSIHandle *handle,bool *is_open)
GSIHandle GSIFrame::GetDeviceHandle(const wxString &dev,bool *is_open) const
{
int devType;
GSIHandle hnd;

    if(is_open)
        *is_open=false;

    //hnd=GSINotAValidHandle;
    wxString s=dev.Upper();     //constant string so return a new one (use dev.MakeUpper() on a non-const string)

    hnd=GetDeviceHandle(s,&devType);
    switch (devType)
    {
    case GSI_DEVICE_RS232:
#if GSI_DUPLICATE_WITH_CONTAINERS
        if(m_deviceHandle_Vec.at(hnd) != GSINotAValidDevHandle)       //id-GSI_ID_COM1 is the offset to comPortArray which stores NodeArray index. ie comport n is attached to Node[index]
            if(is_open)	//can be a NULL pointer if user doesn't want to use parameter
                *is_open=true;
#endif
        if(m_deviceHandle[hnd] != GSINotAValidDevHandle)       //id-GSI_ID_COM1 is the offset to comPortArray which stores NodeArray index. ie comport n is attached to Node[index]
            if(is_open)	//can be a NULL pointer if user doesn't want to use parameter
                *is_open=true;
        break;
    case GSI_DEVICE_USB:
        break;
    case GSI_DEVICE_CAN:            //internal CAN card
        break;
    case GSI_DEVICE_USER:
        break;
    case GSI_DEVICE_NULL:
        break;
    default:
        wxFAIL_MSG("Unknown device type returned from GetDeviceHandle()");
        break;
    }
    return hnd;

}


/************************************************************************
    int GSIFrame::GetDeviceHandle(const wxString &dev, int *dev_type,bool *is_open)
Helper function to obtain a wxID from the device name

    device types:
GSI_DEVICE_RS232,
GSI_DEVICE_USB,
GSI_DEVICE_CAN,            //internal CAN card
GSI_DEVICE_USER,
GSI_DEVICE_NULL
***********************************************************************/
GSIHandle GSIFrame::GetDeviceHandle(const wxString &dev, int *dev_type,bool *is_open) const
{
GSIHandle hnd=0;

    *is_open=false;
    *dev_type=GSI_DEVICE_RS232;
    wxString s=dev.Upper();     //constant string so return a new one (use dev.MakeUpper() on a non-const string)

    hnd=GetDeviceHandle(s,dev_type);
    switch (*dev_type)
    {
    case GSI_DEVICE_RS232:
#if GSI_DUPLICATE_WITH_CONTAINERS
        if(m_deviceHandle_Vec.at(hnd) != GSINotAValidDevHandle)       //id-GSI_ID_COM1 is the offset to comPortArray which stores NodeArray index. ie comport n is attached to Node[index]
            *is_open=true;
#endif
        if(m_deviceHandle[hnd] != GSINotAValidDevHandle)       //id-GSI_ID_COM1 is the offset to comPortArray which stores NodeArray index. ie comport n is attached to Node[index]
            *is_open=true;

        break;
    case GSI_DEVICE_USB:
        break;
    case GSI_DEVICE_CAN:            //internal CAN card
        break;
    case GSI_DEVICE_USER:
        break;
    case GSI_DEVICE_NULL:
        break;
    default:
        wxFAIL_MSG("Unknown device type returned from GetDeviceHandle()");
        break;
    }
    return hnd;

}


/************************************************************************
    int GSIFrame::GetDeviceHandle(const wxString &dev, int *dev_type,bool *is_open)
Helper function to obtain a wxID from the device name

dev_type can be NULL, in which case we ignore, caller only wants the handle
    device types:
GSI_DEVICE_RS232,
GSI_DEVICE_USB,
GSI_DEVICE_CAN,            //internal CAN card
GSI_DEVICE_USER,
GSI_DEVICE_NULL
***********************************************************************/
GSIDevHandle GSIFrame::GetDeviceHandle(const wxString &dev, int *dev_type) const
{
int id=0;


    wxString s=dev.Upper();     //constant string so return a new one (use dev.MakeUpper() on a non-const string)

    if(s=="COM1")
        id= GSI_ID_COM1;
    else if(s=="COM2")
        id= GSI_ID_COM2;
    else if(s=="COM3")
        id= GSI_ID_COM3;
    else if(s=="COM4")
        id= GSI_ID_COM4;
    else if(s=="COM5")
        id= GSI_ID_COM5;
    else if(s=="COM6")
        id= GSI_ID_COM6;

//--------- is it an rs232 device? ------------
    if(id)
    {
        if(dev_type)                        //default to NULL, so can just obtain Handle
            *dev_type=GSI_DEVICE_RS232;
        return (GSIHandle) id-GSI_ID_COM1;

    }


//----------See if usb type--------------------
    if(s=="GSIUSB000")
       id= GSI_ID_USB000;
    else if(s=="GSIUSB001")
        id= GSI_ID_USB001;
    else if(s=="GSIUSB002")
        id=  GSI_ID_USB002;
    else if(s=="GSIUSB003")
        id=  GSI_ID_USB003;
    else if(s=="GSIUSB004")
        id= GSI_ID_USB004;
    else if(s=="GSIUSB005")
        id= GSI_ID_USB005;
    else if(s=="GSIUSB006")
        id=  GSI_ID_USB006;
    else if(s=="GSIUSB007")
        id= GSI_ID_USB007;

//---- is it a usb device ------
    if(id)
    {
        if(dev_type)
            *dev_type=GSI_DEVICE_USB;
        return (GSIHandle) id-GSI_ID_COM1;
    }
//---------not a device type we recognise--------------------
    if(dev_type)
        *dev_type=GSI_DEVICE_NULL;

    return GSINotAValidDevHandle;
}

/**************************************************************
const wxString & GSIFrame::GetDeviceStringId(int id)
****************************************************************/
const wxString GSIFrame::GetDeviceString(int id)
{
wxString s;
    switch (id)
    {
    case GSI_ID_COM1:
        s="COM1";
        break;
    case GSI_ID_COM2:
       s="COM2";
       break;
    case GSI_ID_COM3:
        s="COM3";
        break;
    case GSI_ID_COM4:
        s="COM4";
        break;
    case  GSI_ID_COM5:
        s="COM5";
        break;
    case GSI_ID_COM6:
        s="COM6";
        break;
//----------See if usb type--------------------
    case GSI_ID_USB000:
        s="GSIUSB000";
        break;
    case GSI_ID_USB001:
        s="GSIUSB001";
        break;
    case GSI_ID_USB002:
        s="GSIUSB002";
        break;
    case GSI_ID_USB003:
        s="GSIUSB003";
        break;
    case GSI_ID_USB004:
        s="GSIUSB004";
        break;
    case GSI_ID_USB005:
        s="GSIUSB005";
        break;
    case GSI_ID_USB006:
        s="GSIUSB006";
        break;
    case GSI_ID_USB007:
        s="GSIUSB007";
        break;
    default:
        s="NULL";
        break;
    }

    return(s);
}


/*!int GSIHandle GSIFrame::GetDeviceId(const wxString &dev)
 *Return the id (for use in menus etc.) of a device, given its string
*/
int GSIFrame::GetDeviceId(const wxString &dev)
{

    wxString s=dev.Upper();     //constant string so return a new one (use dev.MakeUpper() on a non-const string)

    if(s=="COM1")
        return GSI_ID_COM1;
    else if(s=="COM2")
        return GSI_ID_COM2;
    else if(s=="COM3")
        return GSI_ID_COM3;
    else if(s=="COM4")
        return GSI_ID_COM4;
    else if(s=="COM5")
        return GSI_ID_COM5;
    else if(s=="COM6")
        return GSI_ID_COM6;

//----------See if usb type--------------------
    if(s=="GSIUSB000")
        return GSI_ID_USB000;
    else if(s=="GSIUSB001")
        return GSI_ID_USB001;
    else if(s=="GSIUSB002")
        return  GSI_ID_USB002;
    else if(s=="GSIUSB003")
        return  GSI_ID_USB003;
    else if(s=="GSIUSB004")
        return GSI_ID_USB004;
    else if(s=="GSIUSB005")
        return GSI_ID_USB005;
    else if(s=="GSIUSB006")
        return  GSI_ID_USB006;
    else if(s=="GSIUSB007")
        return GSI_ID_USB007;


    return -1;
}



/******************************************************************
bool GSIFrame::IsDeviceOpen(int id)
*******************************************************************/
bool GSIFrame::IsDeviceOpen(GSIHandle h_device) const
{
    if(h_device==GSINotAValidDevHandle)
        return false;
    wxASSERT(h_device >=0 && h_device <= GSIMaxNumDevices);

#if GSI_DUPLICATE_WITH_CONTAINERS
    if(m_deviceHandle_Vec.at(h_device) != GSINotAValidDevHandle)
        return true;
#endif
    if(m_deviceHandle[h_device] != GSINotAValidDevHandle)
        return true;

    return false;
}


/**********************************************************************************
errorType GSIFrame::ProcessAsyncRecord(GSIRecord &record,GSINode *node)

GSIDevice (GSISerialThread if using GSIRS232Device) has detected
a record sent from the microcontroller.
The record is in the AsyncQ

We will pass it through the socket as a string. First byte will be an id
must be sure that the ID's don't conflict with wxIDs as the other end of the socket
will be receiving socket events

This function can receive data from multiple nodes/devices. It is not sufficient
to use the default Node()->functions()
The event probably needs to have the node embedded in it
Node is embedded in wxCommandEvent using SetInt()
***********************************************************************************/

errorType GSIFrame::ProcessAsyncRecord(GSIRecord &r,GSINode *node)
{
static int inFunction=0;
errorType rv=errNone;
    inFunction++;
    wxASSERT_MSG(inFunction < 2,"OnProcessAsyncRecord called recursively" );
    TextCtrl()->AppendText(_("Sending Async GSIRecord\n"));
    node->DefaultProcessAsyncRecord(r);      //despatch to correct node object
    if (GetGSIServer().NumClients())
    {
        GSIChar length;
	    r.SetFilledFlag(true);

	    //r.RecordToWxString(s);                      //convert record to str
        GSIChar buff[GSIRecordMaxLength+2];
        length=r.RecordToBuffer(buff);
        //wxString(const char* psz, size_t nLength = wxSTRING_MAXLEN)
        wxString s(buff, length);


        s=(char)GSI_SOCKET_ASYNC_RECORD+s;            //prepend the id
        length++;
	    //int l=s.Length();
	    wxASSERT_MSG(length <GSIRecordMaxLength+2,"Socket data too large");//+1 for zero index, +1 for length byte
	    //SockServer()->GSIClient()->WriteMsg(s,length);
		GetGSIServer().GSIClient()->WriteMsg(s,length);
	    TextCtrl()->AppendText(_("Sent Async GSIRecord\n"));
    }
    else
    {
	    TextCtrl()->AppendText(_("Failed, no socket open.\n"));
    }
    inFunction--;
    return rv;
}

#if 0
/***************************************************************************************************************
errorType GSIFrame::SockCommandOpenDevice(const GSIServerCommandOpenDevice & cmd) //Client has sent OpenDevice command
Don't know how to utilise usb as yet, so will assume that the decice will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present
****************************************************************************************************************/
errorType GSIFrame::SockCommandOpenDevice(GSIServerCommandOpenDevice & cmd) //Client has sent OpenDevice command
{
//GSIHandle hNode;
wxString s;
int id,devType;
bool isOpen;
errorType rv;
GSIHandle hDevice;

    s.Empty();
    const wxString &devStr=cmd.GetDeviceString();

    GSISocketReturnData sr(cmd.GetSock(),GSISocketReturnData::GSI_FAIL);       //tell client we have success/failure

    id=GetDeviceHandle(devStr, &devType,&isOpen);               //get the id of the device we want to open



    s.Printf(_("Opening device %s\n"),devStr);
    TextCtrl()->AppendText(s);

//not sure if this is a smart way to open a device
    if(isOpen)
    {       //don't re-open
        sr.SetReturnCode(GSISocketReturnData:: GSI_ALREADY_OPEN);
        TextCtrl()->AppendText("Device already open\n");
        return errAlreadyOpen;
   }

    if(m_CurrentDevice)
    {
        sr.SetReturnCode(GSISocketReturnData:: GSI_TOO_MANY_OPEN_DEVICES);
        TextCtrl()->AppendText("Too many open devices\n");
        return errAlreadyOpen;
    }

    rv=OpenDevice(cmd.GetDeviceString(), &hDevice);

    if(rv)
    {
        sr.SetReturnCode(GSISocketReturnData:: GSI_OPEN_FAILED);
        TextCtrl()->AppendText("Couldn't open device\n");
        return rv;
    }
    cmd.SetDeviceHandle(hDevice);

    sr.SetReturnCode(GSISocketReturnData:: GSI_SUCCESS);     //will write Success code to client on closing
//Need to send back the node handle (it's GSINode[node] handle)
    wxInt16 * const array=sr.GetDataArray();
    array[0]=hDevice;                                        //return the handle
    //array[1]=hNode;
    sr.SetNumInts(1);
    TextCtrl()->AppendText("Writing return data\n");
    return rv;
}


/***************************************************************************************************************
errorType GSIFrame::SockCommandCloseDevice(const GSICommandCloseDevice & cmd) //Client has sent CloseDevice command
Don't know how to utilise usb as yet, so will assume that the decice will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present
****************************************************************************************************************/
errorType GSIFrame::SockCommandCloseDevice(const GSIServerCommandCloseDevice & cmd) //Client has sent OpenDevice command
{
//const GSIServer sock=GetGSIServer();
wxString s;
int id,devType;
bool isOpen;
errorType rv;

    s.Empty();
    const wxString &devStr=GetDeviceString(cmd.GetDeviceHandle()); //cmd.GetDeviceString();
    GSISocketReturnData sr(cmd.GetSock(),GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure


    s.Printf(_("Closing device %s\n"),devStr);
    TextCtrl()->AppendText(s);

    id=GetDeviceHandle(devStr, &devType,&isOpen);

    if( ! isOpen)
    {
        sr.SetReturnCode(GSISocketReturnData:: GSI_NOT_FOUND);
        TextCtrl()->AppendText("Device already closed\n");
        return errNone;

    }
    rv=CloseDevice(id);
    if(rv)
    {
        sr.SetReturnCode(GSISocketReturnData:: GSI_CLOSE_FAILED);
        return rv;
    }

    sr.SetReturnCode(GSISocketReturnData:: GSI_SUCCESS);     //will write Success code to client on closing
    TextCtrl()->AppendText("Writing return data\n");
    return rv;
}

#endif

#if 0
//------------------------------------------------------------
/*
 *!	GSIFrame::SockCommandAddNode(GSIServerCommandAddNode &cmd)
 *
 *Client has requested we add a node
 */
//------------- GSIFrame::SockCommandAddNode(const GSICommandAddNode &cmd)-----------------------------

errorType   GSIFrame::SockCommandAddNode(GSIServerCommandAddNode &cmd)
{
wxString s;
errorType rv;
GSINodeHandle newNode;
GSIDevHandle hDev;


    const wxString &devStr=GetDeviceString(cmd.GetDeviceHandle()); //cmd.GetDeviceString();
	s.Empty();
    GSISocketReturnData sr(cmd.GetSock(),GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

    s.Printf(_("Add node from microcontroller on device: %s\n"),devStr);

    TextCtrl()->AppendText(s);

	hDev=cmd.GetDeviceHandle();
	newNode=cmd.GetNodeHandle();
    rv=CreateNode(hDev,newNode, &cmd.GetSock());

    if(rv)
	{
		if(rv==errAlreadyOpen)
			sr.SetReturnCode(GSISocketReturnData:: GSI_ALREADY_OPEN);     //will write code to client on closing
		else
			sr.SetReturnCode(GSISocketReturnData:: GSI_FAIL);     //will write code to client on closing
	}
	else
		sr.SetReturnCode(GSISocketReturnData:: GSI_SUCCESS);     //will write Success code to client on closing

    rv=cmd.SetNodeHandle(newNode);

    if(rv)
        sr.SetReturnCode(GSISocketReturnData:: GSI_FAIL);     //will write code to client on closing

    if(sr.GetReturnCode() != GSISocketReturnData:: GSI_SUCCESS)
        rv=errFail;

    TextCtrl()->AppendText("Writing return data\n");

    return rv;
}

#endif

//----------errorType   GSIFrame::SockCommandRemoveNode(const GSIServerCommandRemoveNode &cmd)-------------------

#if 0
/*!
 *errorType   GSIFrame::SockCommandRemoveNode(GSICommandRemoveNode &cmd)
*/
errorType   GSIFrame::SockCommandRemoveNode(GSIServerCommandRemoveNode &cmd)
{
wxInt16 node;
errorType rv;
    GSISocketReturnData sr(cmd.GetSock(),GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

    node=cmd.GetNode();
    rv=RemoveNode(node);
    if(rv==errNone)
        sr.SetReturnCode(GSISocketReturnData:: GSI_SUCCESS);

    return(rv);
}

#endif

errorType GSIFrame::CloseAllNodesOnDevice(GSIHandle device_index)
{
GSINode *gnode;
GSIDevice *d;
wxString s;
wxString devName;

    if(device_index == GSINotAValidDevHandle)
        return errRange;
    devName=GetDeviceString(device_index+GSI_ID_COM1);
    for(int i=0;i<GSIMaxNumNodes;i++)
    {
        gnode=Node(i,false);		//false flag is use_exception_flag, don't for this node
        if(gnode)
        {
            d=gnode->Device();
            if(d)           //NULL means device not open, though node is
            {
                s=d->GetDeviceName();
                s.MakeUpper();
                if(s==devName)
                {
                    RemoveNode(i);
                }
            }
        }
    }
    return errNone;
}


const wxString GSIFrame::GetDeviceString(GSIHandle device_handle)
{
    return GetDeviceString(device_handle+GSI_ID_COM1);
}

/***********************************************************
	const GSIServer & GSIFrame::GetGSIServer() const
************************************************************/
const GSIServer & GSIFrame::GetGSIServer() const
{
	if(m_SockServer==NULL)
	{
		GSI_THROW("No wxServer (GSIFrame::GetGSIServer)",errNoAlloc);
	}
	return *m_SockServer;
}


#if 0
/*!
errorType GSIFrame::OpenDevice(GSICommandOpenDevice &cmd)
*/
errorType GSIFrame::OpenDevice(GSICommandOpenDeviceXXX &cmd)
{
errorType rv;
GSIDevHandle hDev=GSINotAValidDevHandle;

	rv=OpenDevice(cmd.GetDevString(), &hDev);

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nOpenDevice failed on Device:Node:%i Device:%s",err.GetString(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}
	cmd.SetDevHandle(hDev);
	return errNone;
}


/*!
errorType GSIFrame::OpenDevice(GSICommandOpenDevice &cmd)
*/
errorType GSIFrame::OpenDevice(GSICommandOpenDevice &cmd)
{
errorType rv;
GSIDevHandle hDev=GSINotAValidDevHandle;

	rv=OpenDevice(cmd.GetDevString(), &hDev);

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nOpenDevice failed on Device:Node:%i Device:%s",err.GetString(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}
	cmd.SetDevHandle(hDev);
	return errNone;
}
#endif


/*!
	errorType GSIFrame::ProcessGSICommand(GSIServerCommandXXX &Cmd)

Receive polymorphic class and process it.
A server class means that there is still class specific data to be
read from the socket.
The polymorphic Read() function knows how to handle it.
*/

errorType GSIFrame::ProcessServerCommand(GSICommandXXX &Cmd,wxSocketBase &sock)
{
errorType rv;
GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure
wxString s;

#if 0
This code works in VC600 but appears to be broken. The specific polymorphic interface is stripped off in the call to Cmd.xxx
because it isn't const &
If we make it const & we have trouble because the calls DO modify the command string, and textCtrl etc!
#endif
    //wxString t(Cmd.GetCommandString());

	//s.Printf("Reading %s record from socket\n",t.c_str() ); //make a copy of string as we have a const object
//error: passing 'const GSICommandXXX' as 'this' argument of 'void GSICommandXXX::AppendText(const wxString&)'
// discards qualifiers|
    s.Printf("Reading %s record from socket\n",Cmd.GetCommandString().c_str() );
	Cmd.AppendText(s);
	rv=Cmd.Read(sock);
	if(rv)
	{
		sr.SetReturnCode(GSISocketReturnData::GSI_FAIL);
		return errFail;
	}
	rv=Cmd.Execute(*this,sr);	//execute the class specific command

	if (rv)
	{
		sr.SetReturnCode(rv); //automatically converts to GSISocketReturnValue

		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\n%s on Device:Node:%i Device:%s",err.GetString().c_str(),Cmd.GetErrorString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(rv);
	}
	if(sr.GetNumInts()>0)
	{
		wxString s;
		s.Printf("Returning %i 16 bit value(s) to server\n",sr.GetNumInts());
		Cmd.AppendText(s);
	}
	sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
	return errNone;

}


#if 0
/*!
errorType GSIFrame::CloseDevice(GSICommandCloseDevice &cmd)
*/
errorType GSIFrame::CloseDevice(GSICommandCloseDevice &cmd)
{
errorType rv;

	rv=CloseDevice(cmd.GetDeviceHandle());

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nCloseDevice failed on Device:Node:%i Device:%s",err.GetString(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}

	return errNone;
}




/*!
errorType GSIFrame::AddNode(GSICommandAddNode &cmd)
*/
errorType GSIFrame::AddNode(GSICommandAddNode &cmd)
{
wxString s;
GSIDevHandle hDev;
GSINodeHandle newNode;
errorType rv;

    const wxString &devStr=GetDeviceString(cmd.GetDeviceHandle()); //cmd.GetDeviceString();
	s.Empty();

    s.Printf(_("Add node from microcontroller on device: %s\n"),devStr);

    TextCtrl()->AppendText(s);

	hDev=cmd.GetDeviceHandle();
	newNode=cmd.GetNodeHandle();
    //rv=OpenNode(hDev,newNode, &cmd.GetSock());
	rv=CreateNode(hDev,newNode);
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nCreateNode failed on Device:Node:%i Device:%s",err.GetString(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}

    cmd.SetNodeHandle(newNode);

    return rv;
}


/*!
errorType GSIFrame::AddNode(GSICommandAddNodeXXX &cmd)
*/
errorType GSIFrame::AddNode(GSICommandAddNodeXXX &cmd)
{
wxString s;
GSIDevHandle hDev;
GSINodeHandle newNode;
errorType rv;

    const wxString &devStr=GetDeviceString(cmd.GetDeviceHandle()); //cmd.GetDeviceString();
	s.Empty();

    s.Printf(_("Add node from microcontroller on device: %s\n"),devStr);

    TextCtrl()->AppendText(s);

	hDev=cmd.GetDeviceHandle();
	newNode=cmd.GetNodeHandle();
    //rv=OpenNode(hDev,newNode, &cmd.GetSock());
	rv=CreateNode(hDev,newNode);
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nCreateNode failed on Device:Node:%i Device:%s",err.GetString(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}

    cmd.SetNodeHandle(newNode);

    return rv;
}



/*!
errorType GSIFrame::RemoveNode(GSICommandAddNode &cmd)
*/
errorType GSIFrame::RemoveNode(GSICommandRemoveNode &cmd)
{
wxInt16 node;
errorType rv;

    node=cmd.GetNode();
    rv=RemoveNode(node);
    return(rv);

}


#endif


