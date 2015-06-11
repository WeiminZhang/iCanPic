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

/*----------------------------------------------------------------------------------------
Use this code to establish the type of frame to instantiate and initialise

With this method it is possible to have a hierarchy of classes that inherit from
the GSIFrame type. The  'new  xxxFrame' in bool MyApp::OnInit() determines which
class will actually be instantiated.
If required #defines could be used to exclude code from unused frames.
The linker will probably do this in any case
-----------------------------------------------------------------------------------------*/

#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "gsimain.cpp"
#  pragma interface "gsimain.cpp"
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
#include <wx/choicdlg.h>
#include "wx/socket.h"

#include "include/gsi.hpp"
#include "include/gsbexcept.hpp"
#include "include/record.hpp"
#include "include/server.hpp"
#include "include/socket.hpp"


#ifdef STARTLE_SERVER
    #include "startle.hpp"
    #include "STLServer.hpp"

	#include "STLSocketCommands.hpp"
	#include "STLServerCommands.hpp"
    #include "startleFrame.hpp"

#endif

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

// --------------------------------------------------------------------------
// resources
// --------------------------------------------------------------------------

// the application icon
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "mondrian.xpm"
#endif

// Define a new application type
class MyApp : public wxApp
{
public:
  virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)


// ==========================================================================
// implementation
// ==========================================================================

// --------------------------------------------------------------------------
// the application class
// --------------------------------------------------------------------------



bool MyApp::OnInit()
{

  // Create the main application window
  //StartleFrame *frame = new StartleFrame();
#ifdef STARTLE_SERVER
    GSIFrame *frame = DEBUG_NEW StartleFrame();
#else
    GSIFrame *frame = DEBUG_NEW GSIFrame();
#endif
  // Show it and tell the application that it's our main window
  frame->Show(true);
  SetTopWindow(frame);

  // Success
  return true;
}
