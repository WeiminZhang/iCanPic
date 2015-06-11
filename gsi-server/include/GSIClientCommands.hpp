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
#ifndef GSI_CLIENT_COMMANDS_HPP_INCLUDED
#define GSI_CLIENT_COMMANDS_HPP_INCLUDED


#include <gerror.h>
#include "gsi-common/include/gsidefs.h"
#include "gsi-common/include/GSISocketCommands.hpp"

//forward references
class wxSocketBase;



class GSIClientCommandXXX
{
public:
	
    GSIClientCommandXXX(wxSocketBase & sock);  //TextCtrl can be NULL so no reference
    virtual ~GSIClientCommandXXX();
	void	Init();
//Client specific
    virtual errorType ReadServerReturnRecord(); //server returns data, this places it in m_serverReturnXxx variable
	virtual errorType Write(bool flag);      //all derived must know how to write their own data!
	virtual errorType Write(GSICommandXXX & STLCmd,bool flag);      //all derived must know how to write their own data!
	virtual	void DisplayReturnCode(wxTextCtrl *status_text);
    virtual	void DisplayData(wxTextCtrl *status_text);
//Access functions    
	//wxString		GetErrorString() const;
	int				GetReturnCode() const;
    const int       GetReturnNumInts()  const {return (int) m_serverReturnNumInts;}
    wxInt16 * const GetReturnData()     const {return m_serverReturnData;}
	void			SetReadFlag(bool read=true) {m_isRead=read;}
	const bool		IsRead() const {return m_isRead;}

    const bool		IsWritten() const {return m_isWritten;}
    void			SetWrittenFlag(bool written=true)   {m_isWritten=written;}
    void			SetReturnCode(wxInt16 code);
	
	void			SetClientErrorString(const wxString &e_str) {m_ErrorString=e_str;}
	wxString		GetClientErrorString() const {return m_ErrorString;}
	wxSocketBase       & GetSock()     const {return m_Sock;}
	
private:
    //wxTextCtrl      *GetStatusText() const {return m_StatusText;}
    bool        m_isWritten;
	bool		m_isRead;		//ServerReturnRecord has been read
    wxInt16 m_serverReturnCode;
    wxInt16 m_serverReturnNumInts;
    wxInt16 *m_serverReturnData;
	wxString m_ErrorString;
	wxSocketBase &m_Sock;
	//wxTextCtrl*	m_StatusText;

};


//-------GSIClientCommandOpenDeviceXXX----------------------
class GSIClientCommandOpenDeviceXXX : public GSIClientCommandXXX,public GSICommandXXX
{
public:
	GSIClientCommandOpenDeviceXXX(wxSocketBase & sock,const wxString & device,wxTextCtrl * const status_text=NULL);
    virtual ~GSIClientCommandOpenDeviceXXX();
    virtual errorType Write(bool flag=false); //if true the ctor is calling us. If false, don't use m_written

//access functions
		  wxString 		GetDeviceString() const {return m_DeviceStr;}
			void		SetDeviceString(const wxString & s) {m_DeviceStr=s;}
    const GSINodeHandle	GetNodeHandle() const {return(m_hNode);}                         //OpenDevice is implicit OpenNode
    void				SetNodeHandle(GSINodeHandle h_node) {m_hNode=h_node;}

    const GSIDevHandle GetDeviceHandle() const {return m_hDevice;}
    void    SetDeviceHandle(GSIDevHandle hDevice) {m_hDevice=hDevice;}
private:
    wxString    m_DeviceStr;
    GSINodeHandle   m_hNode;
    GSIDevHandle    m_hDevice;               //handle to the device
};


//-------------GSIClientCommandCloseDeviceXXX-----------------
class GSIClientCommandCloseDeviceXXX:public GSIClientCommandXXX,public GSICommandXXX
{
public:
                            
							 GSIClientCommandCloseDeviceXXX(wxSocketBase & sock,GSIDevHandle device_handle,wxTextCtrl * const status_text=NULL);
    virtual                 ~GSIClientCommandCloseDeviceXXX();

    virtual errorType       Write(bool flag=false); //if true the ctor is calling us. If false, don't use m_written
//Access functions
    const wxString        & GetDeviceString() const {return m_DeviceStr;}
    const GSIDevHandle      GetDeviceHandle() const {return m_deviceHandle;}
    errorType               SetDeviceHandle(GSIDevHandle h);
private:
    wxString    m_DeviceStr;
    GSIDevHandle   m_deviceHandle;
    
};



//-----------GSIClientCommandAddNodeXXX-----------------------
class GSIClientCommandAddNodeXXX:public GSIClientCommandXXX, public GSICommandXXX
{
public:
    GSIClientCommandAddNodeXXX(wxSocketBase &sock, GSINodeHandle hNode, GSIDevHandle h_device,wxTextCtrl * const status_text=NULL); //client side
    virtual ~GSIClientCommandAddNodeXXX();
            errorType Write(bool use_written_flag=false);
//Access functions
    virtual const GSINodeHandle GetNodeHandle() const;
    virtual const GSIDevHandle	GetDeviceHandle() const {return m_deviceHandle;}
    errorType					SetNodeHandle(GSINodeHandle h);   
    errorType		            SetDeviceHandle(GSIDevHandle h);
private:
    GSIDevHandle m_deviceHandle;
    GSINodeHandle m_nodeHandle;
};


//-------------GSIClientCommandRemoveNode-------------------------
class GSIClientCommandRemoveNodeXXX : public GSIClientCommandXXX,public GSICommandXXX
{
public:
			GSIClientCommandRemoveNodeXXX(wxSocketBase &sock,GSINodeHandle hNode, GSIDevHandle h_device,wxTextCtrl * const status_text=NULL); //client side
    virtual ~GSIClientCommandRemoveNodeXXX();

            errorType Write(bool use_written_flag=false);
			void SetNodeHandle(GSINodeHandle h);
//Access functions
    virtual const GSINodeHandle GetNode() const;
private:
    GSINodeHandle m_nodeHandle;
};
//--------End  GSIClientCommandXXX



#endif
