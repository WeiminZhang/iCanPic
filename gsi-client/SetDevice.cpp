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
// Name:        SetDevice.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     27/02/2007 17:01:15
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (Personal Edition), 27/02/2007 17:01:15

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "SetDevice.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif


#ifndef WX_PRECOMP
#include "wx/wx.h"
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

////@begin includes
////@end includes

#include "SetDevice.h"

////@begin XPM images
////@end XPM images

/*!
 * SetDevice type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SetDevice, wxDialog )

/*!
 * SetDevice event table definition
 */

BEGIN_EVENT_TABLE( SetDevice, wxDialog )

////@begin SetDevice event table entries
////@end SetDevice event table entries

END_EVENT_TABLE()

/*!
 * SetDevice constructors
 */

SetDevice::SetDevice()
{
    Init();
}

SetDevice::SetDevice( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * SetDevice creator
 */

bool SetDevice::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin SetDevice creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end SetDevice creation
    return true;
}

/*!
 * SetDevice destructor
 */

SetDevice::~SetDevice()
{
////@begin SetDevice destruction
////@end SetDevice destruction
}

/*!
 * Member initialisation 
 */

void SetDevice::Init()
{
////@begin SetDevice member initialisation
    m_Choice = NULL;
////@end SetDevice member initialisation
}
/*!
 * Control creation for SetDevice
 */

void SetDevice::CreateControls()
{    
////@begin SetDevice content construction
    // Generated by DialogBlocks, 12/03/2007 16:16:49 (Personal Edition)

    SetDevice* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Select device"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxString m_ChoiceStrings[] = {
        _("Com1"),
        _("Com2"),
        _("Com3"),
        _("Com4"),
        _("Com5"),
        _("Com6"),
        _("USB000"),
        _("USB001"),
        _("USB002"),
        _("USB003"),
        _("USB004"),
        _("USB005"),
        _("USB006"),
        _("USB007"),
        _("USB008"),
        _("USB009")
    };
    m_Choice = new wxChoice( itemDialog1, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 16, m_ChoiceStrings, 0 );
    m_Choice->SetStringSelection(_("Com1"));
    m_Choice->SetHelpText(_("Set a device name that hardware nodes are attached to."));
    if (ShowToolTips())
        m_Choice->SetToolTip(_("Set a device name that hardware nodes are attached to."));
    itemStaticBoxSizer3->Add(m_Choice, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end SetDevice content construction
}

/*!
 * Should we show tooltips?
 */

bool SetDevice::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap SetDevice::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin SetDevice bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end SetDevice bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon SetDevice::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin SetDevice icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end SetDevice icon retrieval
}
