
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
#ifndef GSITEST_HPP_INCLUDED
#define GSITEST_HPP_INCLUDED

#include "client.hpp"

//forward definitions
class GSISocketClientStatusBar;
class GSISocketClient;

// --------------------------------------------------------------------------
// classes
// --------------------------------------------------------------------------

// Define a new application type
class MyApp : public wxApp
{
public:
  virtual bool OnInit();
};


class GSITestPanel: public wxPanel
{
public:
    GSITestPanel(wxFrame *frame, int x, int y, int w, int h);
    virtual ~GSITestPanel();

private:
    wxTextCtrl *m_text;
    wxTextCtrl *m_text1;
    wxLog * m_logTargetOld ;

    wxBookCtrl  *m_book;
};

/*********************************************************
		class GSITestFrame
**********************************************************/

// Define a new frame type: this is going to be our main frame
class GSITestFrame : public wxFrame
{
public:
  GSITestFrame();
  ~GSITestFrame();

  // event handlers for File menu
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    // event handlers for Socket menu
    virtual void OnOpenConnection(wxCommandEvent& event);

    virtual void OnTest1(wxCommandEvent& event);
    virtual void OnTest2(wxCommandEvent& event);
    virtual void OnTest3(wxCommandEvent& event);
    virtual void OnTest4(wxCommandEvent& event);
    virtual void OnDebugSocket(wxCommandEvent& event);
    //void OnSocketEvent(wxSocketEvent& event);
    //void OnPushEvent(wxCommandEvent& event);
    virtual void OnCloseConnection(wxCommandEvent& event);
    virtual void CloseConnection();                             //OnCloseConnection() calls this.
    //custom event handler
    //void GSITestFrame::OnProcessSocketEvent(wxCommandEvent& event);
    virtual void OnGSISocketEvent(wxCommandEvent& event);
    virtual void SetBusyFlag(bool flag)  {m_busy=flag;}

    // 1st-level exception handling: we overload ProcessEvent() to be able to
    // catch exceptions which occur in MyFrame methods here
    virtual bool ProcessEvent(wxEvent& event);

    bool GetBusyFlag()  {return m_busy;}
    //void                 UpdateStatusBar()   {m_StatusBar->Update();}
    void DisplayReturnCode();

#if wxUSE_URL
  // event handlers for Protocols menu
  void OnTestURL(wxCommandEvent& event);
#endif

  // event handlers for DatagramSocket menu (stub)
  void OnDatagram(wxCommandEvent& event);

  // socket event handler
  //void OnSocketEvent(wxSocketEvent& event);

  // convenience functions
  //void UpdateStatusBar();
	//GSISocketClient	* const GetGSISocketClient() const {return m_Client;}
    GSISocketClient   &       GetGSISocketClient()   const {return *m_Client;}

	wxMenu	*	const		GetSocketMenu() const {return m_menuSocket;}
	bool					GetBusyFlag()		const {return m_busy;}
    wxTextCtrl  &           GetTextCtrl() const {return *m_TextCtrl;}

    const       wxNotebook *GetNotebook() const {return m_Notebook;}
    //GSITestPanel *GetPanel() const { return m_Panel; }
    wxPanel             *GetPanel() const { return m_Panel; }
protected:
    // number of pushed event handlers
    //size_t m_nPush;
	wxTextCtrl	* const TextCtrl() const {return m_TextCtrl;}
	GSISocketClientStatusBar * const SockClientStatusBar() const {return m_StatusBar;}
    wxMenu         *m_menuDebugSocket;
private:

	GSISocketClient			 *m_Client;
	wxTextCtrl     *m_TextCtrl;
	GSISocketClientStatusBar	 *m_StatusBar;
    wxNotebook      *m_Notebook;
    wxMenu          *m_menuFile;
    wxMenu          *m_menuSocket;
    wxMenu          *m_menuDatagramSocket;
    wxMenu          *m_menuProtocols;
    //wxMenu         *m_menuDebugSocket;      //make a protected element so derived classes can add to this menu
    wxMenuBar       *m_menuBar;
    bool             m_busy;
    //GSITestPanel    *m_Panel;
    wxPanel         *m_Panel;

    wxBoxSizer      *m_sizerFrame;

  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()
};


#if DIALOG_BLOCKS
#else
class GSISocketClient;
/*********************************************************
		class GSITestClient
**********************************************************/
class GSITestClient : public GSISocketClient
{
public:
	//GSITestClient(wxEvtHandler * evt,GSISocketClientStatusBar *status_bar=NULL,wxTextCtrl *text_ctrl=NULL);
    GSITestClient(GSITestFrame * frame,GSISocketClientStatusBar *status_bar=NULL,wxTextCtrl *text_ctrl=NULL);
	//void OnTest(wxCommandEvent& event);
protected:
		wxStatusBar * const StausBar() const{return m_StatusBar;}
		wxTextCtrl	* const	TextCtrl() const {return(m_TextCtrl);}
			//wxSocketClient *m_Sock;
private:
		wxStatusBar *m_StatusBar;
		wxTextCtrl *m_TextCtrl;
        GSITestFrame *m_GSITestFrame;
};

class GSISocketClientStatusBar: public wxStatusBar
{
public:
	GSISocketClientStatusBar(GSITestFrame *gsi_frame,wxWindowID id);

	~GSISocketClientStatusBar();
	virtual void Update();
private:
		GSITestFrame  * const m_GSITestFrame;

};

#endif  //#if DIALOG_BLOCKS

// IDs for the controls and the menu commands
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
#ifdef STARTLE_SERVER
    STARTLE_TEST1,
#endif
    CLIENT_CLOSE,
#if wxUSE_URL
    CLIENT_TESTURL,
#endif

    CLIENT_DGRAM,

    CLIENT_DEBUG,
    CLIENT_DEBUG_USER1,
	CLIENT_DEBUG_USER2,
	CLIENT_DEBUG_USER3,
	CLIENT_DEBUG_USER4,
	CLIENT_DEBUG_USER5,
  // id for socket
  //SOCKET_ID
};


#endif
