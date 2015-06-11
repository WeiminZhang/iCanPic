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
#ifndef CLIENT_HPP_INCLUDED
#define CLIENT_HPP_INCLUDED

#include<wx\socket.h>
#include <gerror.h>
#include "gsi-common/include/gsidefs.h"
/**********************************************************************************
GSISocketClient should be a stand-alone class. It must not require any knowledge of the
front-end gui. It is designed so that it can be (hopefully) used in a dll/shared
library using the users choice of programming language. 
Alternatively, user code must implement the functionality of this class

For the example client gui, we derive a class from GSISocketClient.

***********************************************************************************/

class GSISocketClient : public wxEvtHandler, public wxSocketClient        //wxEvtHandler MUST be first class inherited
{
public:
		explicit GSISocketClient(wxEvtHandler & evt,wxTextCtrl *text_ctrl = NULL);  //DB change
		virtual ~GSISocketClient();
		virtual void OnSocketEvent(wxSocketEvent& event);

        errorType ConnectToServer(int sock_address, const wxString & host);
        errorType CloseSocket();

		virtual bool IsOpen() const {return m_open;}

        const int   GetNode() const {return m_node;}
        void 		SetNodeHandle(GSINodeHandle node);

        errorType   OpenDevice();
        void        AppendText(const wxString & text);               //Write to text control if one has been passed


 private:
		bool m_open;
		bool m_busy;
		wxEvtHandler & m_EventHandler;
        GSINodeHandle m_node;                             //node this 
        wxTextCtrl *m_TextCtrl;
    DECLARE_EVENT_TABLE()
};


#if DIALOG_BLOCKS

//forward reference

class GSISocketClientTestFrame;

class GSISocketClientStatusBar: public wxStatusBar
{
public:
    explicit GSISocketClientStatusBar(const GSISocketClientTestFrame &gsi_frame,wxWindowID id);
	~GSISocketClientStatusBar();
	virtual void Update();
private:
		GSISocketClientTestFrame  * const m_GSISocketClientTestFrame;

};



/*********************************************************
		class GSITestClient
**********************************************************/
class GSITestClient : public GSISocketClient
{
public:
    explicit GSITestClient(wxEvtHandler & evt_handler,wxTextCtrl * const text_ctrl=NULL);

protected:
	wxStatusBar * const GetStatusBar() const{return m_StatusBar;}       
	wxTextCtrl	* const	GetStatusTextCtrl() const {return(m_TextCtrl);}      //don't use a reference as this can be NULL

private:

		wxStatusBar *m_StatusBar;
		wxTextCtrl *m_TextCtrl;                                             //can be NULL so no reference
        GSISocketClientTestFrame *m_GSISocketClientTestFrame;
};

#endif //DIALOG_BLOCKS


#endif
