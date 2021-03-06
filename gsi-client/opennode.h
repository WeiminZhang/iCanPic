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
// Name:        opennode.h
// Purpose:     
// Author:      Glenn Self
// Modified by: 
// Created:     22/03/2007 12:06:47
/////////////////////////////////////////////////////////////////////////////

#ifndef _OPENNODEDIALOG_H_
#define _OPENNODEDIALOG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "opennodedialog.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
#include "wx/spinctrl.h"
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
#define GSI_ID_CLIENT_TEST_OPEN_NODE_DIALOG 10023
#define GSI_ID_CLIENT_TEST_SPINCTRL1 10024
#define SYMBOL_OPENNODEDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxDIALOG_MODAL
#define SYMBOL_OPENNODEDIALOG_TITLE _("OpenNodeDialog")
#define SYMBOL_OPENNODEDIALOG_IDNAME GSI_ID_CLIENT_TEST_OPEN_NODE_DIALOG
#define SYMBOL_OPENNODEDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_OPENNODEDIALOG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * OpenNodeDialog class declaration
 */

class OpenNodeDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( OpenNodeDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    OpenNodeDialog();
    OpenNodeDialog( wxWindow* parent, wxWindowID id = SYMBOL_OPENNODEDIALOG_IDNAME, const wxString& caption = SYMBOL_OPENNODEDIALOG_TITLE, const wxPoint& pos = SYMBOL_OPENNODEDIALOG_POSITION, const wxSize& size = SYMBOL_OPENNODEDIALOG_SIZE, long style = SYMBOL_OPENNODEDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_OPENNODEDIALOG_IDNAME, const wxString& caption = SYMBOL_OPENNODEDIALOG_TITLE, const wxPoint& pos = SYMBOL_OPENNODEDIALOG_POSITION, const wxSize& size = SYMBOL_OPENNODEDIALOG_SIZE, long style = SYMBOL_OPENNODEDIALOG_STYLE );

    /// Destructor
    ~OpenNodeDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin OpenNodeDialog event handler declarations

////@end OpenNodeDialog event handler declarations

////@begin OpenNodeDialog member function declarations

    int GetNode() const { return m_node ; }
    void SetNode(int value) { m_node = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end OpenNodeDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin OpenNodeDialog member variables
private:
    int m_node;
////@end OpenNodeDialog member variables
};

#endif
    // _OPENNODE_H_
