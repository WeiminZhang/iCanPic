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
#ifndef SERVER_HPP_INCLUDED
#define SERVER_HPP_INCLUDED

#include <gerror.h>

//forward references
class wxSocketEvent;
class wxSocketBase;
class wxSocketServer;

class GSIFrame ;


class GSIServer : public wxEvtHandler
{
public:
    //GSIServer(const GSINode &node,unsigned short sock_address);
	GSIServer(unsigned short sock_address,wxStatusBar *status_bar=NULL,wxTextCtrl *text_ctrl=NULL,GSIFrame *gsi_frame=NULL);
    virtual ~GSIServer();
    //void OnTest(wxCommandEvent& event);
	virtual void OnSocketEvent(wxSocketEvent& event);
	virtual void OnServerEvent(wxSocketEvent& event);

	wxSocketBase *	const GSIClient() const {return(m_GSIClient);}		//one and only one client so far. Refuse connection to more than one
	int     	const NumClients() const {return m_numClients;}
	GSIFrame &	GetGSIFrame() const {return *m_GSIFrame;}
	//errorType Open(unsigned short addr);
    //ConnectToServer(unsigned short sock_address);
	virtual void Test1(wxSocketBase *sock);
	virtual void Test2(wxSocketBase *sock);
	virtual void Test3(wxSocketBase *sock);
	virtual errorType ProcessRecordFromClient(wxSocketBase &sock);
	virtual errorType ProcessUser1Record(wxSocketBase &sock);
	virtual errorType ProcessUser2Record(wxSocketBase &sock);
	virtual errorType ProcessUser3Record(wxSocketBase &sock);
	virtual errorType ProcessUser4Record(wxSocketBase &sock);
	virtual errorType ProcessUser5Record(wxSocketBase &sock);
	virtual errorType ProcessUser6Record(wxSocketBase &sock);
	virtual errorType ProcessUser7Record(wxSocketBase &sock);
	virtual errorType ProcessUser8Record(wxSocketBase &sock);

	bool IsOpen() const {return m_open;}
	wxSocketServer * const Server() const {return(m_Server);}
protected:
	wxStatusBar * const StatusBar() const{return m_StatusBar;}
	wxTextCtrl	* TextCtrl() const {return(m_TextCtrl);}
	void	UpdateStatusBar();
private:
    int m_numClients;
   	wxSocketBase *m_GSIClient;							//the address of the GSIClient socket

    wxSocketServer	*m_Server;
	wxStatusBar *m_StatusBar;
	wxTextCtrl *m_TextCtrl;
    bool m_open;
    bool m_busy;
    GSIFrame *m_GSIFrame;
		//const GSINode	 & m_Node;
	bool m_dummyTextCtrlFlag;	//if user doesn't supply a ctrl, insantiate a dummy
    DECLARE_EVENT_TABLE()
};



#endif //SERVER_HPP_INCLUDED

