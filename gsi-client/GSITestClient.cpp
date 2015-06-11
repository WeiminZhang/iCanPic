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
/////////////////////////////////////////////////////////////////////////////
// Name:        GSITestClient.cpp
// Purpose:
// Author:
// Modified by:
// Created:     28/02/2007 12:04:14
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "GSITestClient.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "GSITestClient.h"
#include "StartleTest.hpp"

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

////@begin XPM images
////@end XPM images

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( GSITesClient )
////@end implement app

/*!
 * GSITesClient type definition
 */

IMPLEMENT_CLASS( GSITesClient, wxApp )

/*!
 * GSITesClient event table definition
 */

BEGIN_EVENT_TABLE( GSITesClient, wxApp )

////@begin GSITesClient event table entries
////@end GSITesClient event table entries

END_EVENT_TABLE()

/*!
 * Constructor for GSITesClient
 */

GSITesClient::GSITesClient()
{
    Init();
}

/*!
 * Member initialisation
 */

void GSITesClient::Init()
{
////@begin GSITesClient member initialisation
////@end GSITesClient member initialisation
}
/*!
 * Initialisation for GSITesClient
 */

bool GSITesClient::OnInit()
{

#if wxUSE_XPM
    wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
    wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
    wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
    wxImage::AddHandler(new wxGIFHandler);
#endif

#ifdef STARTLE_SERVER
	//GSIClientTestFrame *mainWindow = DEBUG_NEW STLTestFrame(NULL, GSI_ID_CLIENT_TESTFRAME);
	GSIClientTestFrame *mainWindow = DEBUG_NEW STLClientTestFrame(NULL, GSI_ID_CLIENT_TESTFRAME);
	mainWindow->Show(true);
#else
    GSIClientTestFrame *mainWindow = DEBUG_NEW GSIClientTestFrame(NULL, GSI_ID_CLIENT_TESTFRAME);
    mainWindow->Show(true);
#endif

    //GSIClientTestFrame* mainWindow = new GSIClientTestFrame( NULL, GSI_ID_CLIENT_TESTFRAME );
    //mainWindow->Show(true);



    return true;
}

/*!
 * Cleanup for GSITesClient
 */
int GSITesClient::OnExit()
{
////@begin GSITesClient cleanup
	return wxApp::OnExit();
////@end GSITesClient cleanup
}

