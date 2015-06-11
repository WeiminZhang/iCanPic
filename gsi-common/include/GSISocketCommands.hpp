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
#ifndef GSI_SOCKET_COMMANDS_HPP_INCLUDED
#define GSI_SOCKET_COMMANDS_HPP_INCLUDED


#include <gerror.h>
#include "gsidefs.h"
#include "GSICommands.hpp"
//forward references
class wxSocketBase;
class GSIReturnValue;

const size_t GSICommandArrayMaxElements=GSIRecordMaxLength+1;        //+1 for the inital flag

enum GSISockCommands
{
GSI_COMMAND_OPEN_DEVICE=1,
GSI_COMMAND_CLOSE_DEVICE,
GSI_COMMAND_ADD_NODE,
GSI_COMMAND_REMOVE_NODE

};


class GSISocketXXX
{
public:
    GSISocketXXX(wxSocketBase & sock);  //TextCtrl can be NULL so no reference
    virtual ~GSISocketXXX();
//Access functions
	void				SetReadFlag(bool flag) {m_isReadFlag=flag;}
	bool				IsRead() {return m_isReadFlag;}
    wxSocketBase       & GetSock()     const {return m_Sock;}
private:
	bool	m_isReadFlag;
    wxSocketBase &m_Sock;

};

class GSISocketCommandXXX : public GSICommandXXX
{
public:
    GSISocketCommandXXX(wxSocketBase & sock,wxTextCtrl * const status_text=NULL);  //TextCtrl can be NULL so no reference
    virtual ~GSISocketCommandXXX();

//Access functions
    wxSocketBase       & GetSock()     const {return m_Sock;}
private:
    wxSocketBase &m_Sock;

};




#endif


