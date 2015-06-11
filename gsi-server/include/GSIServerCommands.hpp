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
#ifndef GSI_SERVER_COMMANDS_HPP_INCLUDED
#define GSI_SERVER_COMMANDS_HPP_INCLUDED


#include <gerror.h>
#include "gsidefs.h"
#include "GSISocketCommands.hpp"

//forward references
class wxSocketBase;



/*!
class GSIServerCommandXXX : public GSISocketCommandXXX
*/

class GSIServerCommandXXX : public GSICommandXXX
{
public:

    GSIServerCommandXXX(wxSocketBase & sock);
    virtual ~GSIServerCommandXXX();

	void Init();
    virtual  errorType Read();  //read the basic GSIRecord parameters

//Access functions
    void				SetReadFlag(bool read=true)         {m_isRead=read;}

    const bool			IsRead ()   const {return m_isRead;}
	wxSocketBase       & GetSock()     const {return m_Sock;}

private:
    bool        m_isRead;
	wxSocketBase &m_Sock;
};



/*!
-------GSIServerCommandOpenDeviceXXX----------------------
	ctor

Will be able to send GSICommand & or GSIServerCommand& to other functions or
the virtual functions will provide customisation of behaviour
*/
class GSIServerCommandOpenDeviceXXX:public GSICommandOpenDeviceXXX,GSISocketXXX
{
public:
//The send_now flag is true for a client and false for a server
    GSIServerCommandOpenDeviceXXX(wxSocketBase & sock);
    virtual ~GSIServerCommandOpenDeviceXXX();
    //errorType Read();
    errorType	Read(wxSocketBase &sock); //read from Socket


	errorType Execute(GSIFrame &Frame,GSISocketReturnData &sr);	//execute the class specific command
private:

};



//-------------GSIServerCommandCloseDeviceXXX-----------------
class GSIServerCommandCloseDeviceXXX:public GSICommandCloseDeviceXXX,GSISocketXXX
{
public:
						     GSIServerCommandCloseDeviceXXX(wxSocketBase & sock); //server side ctor
    virtual                 ~GSIServerCommandCloseDeviceXXX();
    //virtual errorType       Read();
    errorType	Read(wxSocketBase &sock); //read from Socket

	errorType				Execute(GSIFrame &Frame,GSISocketReturnData &sr);	//execute the class specific command
private:
};

//--------------End GSICommandCloseDeviceXXX class definition-----------------



//-----------GSICommandAddNodeXXX-----------------------
class GSIServerCommandAddNodeXXX:public GSIServerCommandXXX
{
public:
    GSIServerCommandAddNodeXXX(wxSocketBase &sock); //server side
    virtual ~GSIServerCommandAddNodeXXX();
    //virtual errorType Read();
    errorType	Read(wxSocketBase &sock); //read from Socket
    virtual const GSINodeHandle GetNodeHandle() const;
    virtual const GSIDevHandle GetDeviceHandle() const {return m_deviceHandle;}
    errorType               SetNodeHandle(GSINodeHandle h);
    errorType               SetDeviceHandle(GSIDevHandle h);
	errorType				Execute(GSIFrame &Frame,GSISocketReturnData &sr);
private:
    GSIDevHandle m_deviceHandle;
    GSINodeHandle m_nodeHandle;
};

//--------End GSICommandAddNodeXXX class definition-----------

//-------------GSICommandRemoveNodeXXX-------------------------
class GSIServerCommandRemoveNodeXXX:public GSIServerCommandXXX
{
public:
    GSIServerCommandRemoveNodeXXX(wxSocketBase &sock); //server side

    virtual ~GSIServerCommandRemoveNodeXXX();
    //virtual errorType Read();                   //Usually called by the server to read from the client
    errorType	Read(wxSocketBase &sock); //read from Socket
    virtual const GSINodeHandle GetNodeHandle() const;
	errorType Execute(GSIFrame &Frame,GSISocketReturnData &sr);
private:
    GSINodeHandle m_nodeHandle;
};
// End GSIServerCommandRemoveNodeXXX class definition -------------




#endif
