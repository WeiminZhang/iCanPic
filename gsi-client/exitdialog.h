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
// Name:        exitdialog.h
// Purpose:     
// Author:      Glenn Self
// Modified by: 
// Created:     22/03/2007 12:36:32
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (Personal Edition), 22/03/2007 12:36:32

#ifndef _EXITDIALOG_H_
#define _EXITDIALOG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "exitdialog.h"
#endif

/*!
 * Includes
 */

////@begin includes
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
#define ID_EXIT_DIALOG 10005
#define ID_RADIOBOX1 10006
#define SYMBOL_EXITDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_EXITDIALOG_TITLE _("Exit")
#define SYMBOL_EXITDIALOG_IDNAME ID_EXIT_DIALOG
#define SYMBOL_EXITDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_EXITDIALOG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * ExitDialog class declaration
 */

class ExitDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( ExitDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ExitDialog();
    ExitDialog( wxWindow* parent, wxWindowID id = SYMBOL_EXITDIALOG_IDNAME, const wxString& caption = SYMBOL_EXITDIALOG_TITLE, const wxPoint& pos = SYMBOL_EXITDIALOG_POSITION, const wxSize& size = SYMBOL_EXITDIALOG_SIZE, long style = SYMBOL_EXITDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EXITDIALOG_IDNAME, const wxString& caption = SYMBOL_EXITDIALOG_TITLE, const wxPoint& pos = SYMBOL_EXITDIALOG_POSITION, const wxSize& size = SYMBOL_EXITDIALOG_SIZE, long style = SYMBOL_EXITDIALOG_STYLE );

    /// Destructor
    ~ExitDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin ExitDialog event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOKClick( wxCommandEvent& event );

////@end ExitDialog event handler declarations

////@begin ExitDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ExitDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ExitDialog member variables
    wxRadioBox* m_RadioBox;
////@end ExitDialog member variables
};

#endif
    // _EXITDIALOG_H_