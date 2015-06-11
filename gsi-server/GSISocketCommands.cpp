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
#  pragma implementation "GSISocketCommands.cpp"
#  pragma interface "GSISocketCommands.cpp"
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
#endif


//This class now does nothing. Could just as easily inherit behaviour
//directly from wxSocketBase
//However, leave it here as we might wish to add functions later

/**************************************************************************************************
    ctor
GSISocketXXX::GSISocketXXX(wxSocketBase & sock)
**************************************************************************************************/
GSISocketXXX::GSISocketXXX(wxSocketBase & sock)
:m_Sock(sock)
{
	GetSock().SetTimeout(10);		//by default set all socket IO to 10 sec timeout
}



/**************************************************************************************************
    dtor
GSISocketCommand::~GSISocketCommand()
**************************************************************************************************/
GSISocketXXX::~GSISocketXXX()
{

}
 


