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
// Name:        GSITestClient.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     28/02/2007 12:04:14
/////////////////////////////////////////////////////////////////////////////

#ifndef _GSITESTCLIENT_H_
#define _GSITESTCLIENT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "GSITestClient.h"
#endif

#define DB 1                //this code uses DialogBlocks

/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "GSIClientTestframe.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * GSITesClient class declaration
 */

class GSITesClient: public wxApp
{    
    DECLARE_CLASS( GSITesClient )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    GSITesClient();

    /// Initialises member variables
    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin GSITesClient event handler declarations

////@end GSITesClient event handler declarations

////@begin GSITesClient member function declarations

////@end GSITesClient member function declarations

////@begin GSITesClient member variables
////@end GSITesClient member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(GSITesClient)
////@end declare app

#endif
    // _GSITESTCLIENT_H_
