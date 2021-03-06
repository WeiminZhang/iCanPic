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
// Name:        GSIClientTestPanel.cpp
// Purpose:
// Author:
// Modified by:
// Created:     27/02/2007 17:01:15
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (Personal Edition), 27/02/2007 17:01:15

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "GSIClientTestPanel.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include<vector>
using namespace std;
//#include "gsi-common/include/gsi.hpp"


////@begin includes
#include "GSIClientTestframe.h"
////@end includes

#include "GSIClientTestPanel.h"

////@begin XPM images
////@end XPM images

/*!
 * GSITestClientPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( GSITestClientPanel, wxPanel )

/*!
 * GSITestClientPanel event table definition
 */

BEGIN_EVENT_TABLE( GSITestClientPanel, wxPanel )

////@begin GSITestClientPanel event table entries
    EVT_COMBOBOX( GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD, GSITestClientPanel::OnGsiIdClientTestNbComboboxLcdSelected )
    EVT_TEXT_ENTER( GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD, GSITestClientPanel::OnGsiIdClientTestNbComboboxLcdEnter )

    EVT_COMBOBOX( GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL, GSITestClientPanel::OnGsiIdClientTestNbComboBoxAllSelected )
    EVT_TEXT_ENTER( GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL, GSITestClientPanel::OnGsiIdClientTestNbComboBoxAllEnter )

    EVT_BUTTON( GSI_ID_CLIENT_TEST_KD_BUTTON1, GSITestClientPanel::OnGsiIdClientTestKdButton1Click )

////@end GSITestClientPanel event table entries

END_EVENT_TABLE()

/*!
 * GSITestClientPanel constructors
 */

GSITestClientPanel::GSITestClientPanel()
{
    Init();
}

GSITestClientPanel::GSITestClientPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

/*!
 * GSITESTClientPanel creator
 */

bool GSITestClientPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin GSITestClientPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
////@end GSITestClientPanel creation
    return true;
}

/*!
 * GSITESTClientPanel destructor
 */

GSITestClientPanel::~GSITestClientPanel()
{
////@begin GSITestClientPanel destruction
////@end GSITestClientPanel destruction
}

/*!
 * Member initialisation
 */

void GSITestClientPanel::Init()
{
////@begin GSITestClientPanel member initialisation
    m_SplitterWindow = NULL;
    m_Panel = NULL;
    m_TextCtrlStatus = NULL;
////@end GSITestClientPanel member initialisation
}
/*!
 * Control creation for GSITESTClientPanel
 */

void GSITestClientPanel::CreateControls()
{
////@begin GSITestClientPanel content construction
    // Generated by DialogBlocks, 28/02/2007 16:12:33 (Personal Edition)

    GSITestClientPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_SplitterWindow = new wxSplitterWindow( itemPanel1, GSI_ID_CLIENT_TEST_SPLITTERWINDOW, wxDefaultPosition, wxSize(100, 100), wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER );
    m_SplitterWindow->SetMinimumPaneSize(0);

    m_Panel = new wxPanel( m_SplitterWindow, GSI_ID_CLIENT_TEST_NB_PANEL0, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    m_Panel->SetSizer(itemBoxSizer5);

    wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(m_Panel, wxID_ANY, _("Enter GSI commands"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxHORIZONTAL);
    itemBoxSizer5->Add(itemStaticBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText9 = new wxStaticText( m_Panel, GSI_ID_CLIENT_TEST_NB_STATIC_LCD, _("LCD commands"), wxDefaultPosition, wxDefaultSize, 0 );
    if (ShowToolTips())
        itemStaticText9->SetToolTip(_("Enter commands to display on node's LCD"));
    itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALL, 5);

    wxString* itemComboBox10Strings = NULL;
    wxComboBox* itemComboBox10 = new wxComboBox( m_Panel, GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD, _T(""), wxDefaultPosition, wxDefaultSize, 0, itemComboBox10Strings, wxCB_DROPDOWN );
    itemBoxSizer8->Add(itemComboBox10, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticText* itemStaticText12 = new wxStaticText( m_Panel, wxID_STATIC, _("All commands"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALL, 5);

    wxString* itemComboBox13Strings = NULL;
    wxComboBox* itemComboBox13 = new wxComboBox( m_Panel, GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL, _T(""), wxDefaultPosition, wxDefaultSize, 0, itemComboBox13Strings, wxCB_DROPDOWN );
    itemBoxSizer11->Add(itemComboBox13, 0, wxGROW|wxALL, 5);

    itemBoxSizer5->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer15, 0, wxALIGN_LEFT|wxALL, 5);
    itemBoxSizer15->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(m_Panel, wxID_ANY, _("Keypad"));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemBoxSizer15->Add(itemStaticBoxSizer17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer18, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton19 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON1, _("1"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton19->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer18->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON2, _("2"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton20->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer18->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton21 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON3, _("3"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton21->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer18->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer22, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton23 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON4, _("4"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton23->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer22->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton24 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON5, _("5"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton24->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer22->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton25 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON6, _("6"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton25->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer22->Add(itemButton25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer26, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton27 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON7, _("7"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton27->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer26->Add(itemButton27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton28 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON8, _("8"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton28->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer26->Add(itemButton28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton29 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON9, _("9"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton29->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer26->Add(itemButton29, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer30 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer30, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton31 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON10, _("0"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton31->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer30->Add(itemButton31, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton32 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON11, _("*"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton32->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer30->Add(itemButton32, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton33 = new wxButton( m_Panel, GSI_ID_CLIENT_TEST_KD_BUTTON12, _("#"), wxDefaultPosition, wxSize(30, -1), 0 );
    itemButton33->SetForegroundColour(wxColour(255, 0, 0));
    itemBoxSizer30->Add(itemButton33, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer5->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer35Static = new wxStaticBox(m_Panel, wxID_ANY, _("LCD (on node)"));
    wxStaticBoxSizer* itemStaticBoxSizer35 = new wxStaticBoxSizer(itemStaticBoxSizer35Static, wxVERTICAL);
    itemBoxSizer5->Add(itemStaticBoxSizer35, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxTextCtrl* itemTextCtrl36 = new wxTextCtrl( m_Panel, GSI_ID_CLIENT_TEST_NB_TEXTCTRL_LCD, _("012356789012345"), wxDefaultPosition, wxSize(300, 50), wxTE_READONLY );
    itemStaticBoxSizer35->Add(itemTextCtrl36, 0, wxGROW|wxALL, 5);

    itemBoxSizer5->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_TextCtrlStatus = new wxTextCtrl( m_SplitterWindow, GSI_ID_CLIENT_TEST_NB_TEXTCTRL_CLIENT, _T(""), wxDefaultPosition, wxSize(90, 100), wxTE_MULTILINE|wxNO_BORDER );

    m_SplitterWindow->SplitHorizontally(m_Panel, m_TextCtrlStatus, 600);
    itemBoxSizer2->Add(m_SplitterWindow, 1, wxGROW|wxALL, 5);

////@end GSITestClientPanel content construction
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD
 */

void GSITestClientPanel::OnGsiIdClientTestNbComboboxLcdSelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_COMBOBOX_SELECTED event handler for GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD in GSITESTClientPanel.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_COMBOBOX_SELECTED event handler for GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD in GSITESTClientPanel.
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD
 */

void GSITestClientPanel::OnGsiIdClientTestNbComboboxLcdEnter( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_TEXT_ENTER event handler for GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD in GSITESTClientPanel.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_TEXT_ENTER event handler for GSI_ID_CLIENT_TEST_NB_COMBOBOX_LCD in GSITESTClientPanel.
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL
 */

void GSITestClientPanel::OnGsiIdClientTestNbComboBoxAllSelected( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_COMBOBOX_SELECTED event handler for GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL in GSITESTClientPanel.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_COMBOBOX_SELECTED event handler for GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL in GSITESTClientPanel.
}

/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL
 */

void GSITestClientPanel::OnGsiIdClientTestNbComboBoxAllEnter( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_TEXT_ENTER event handler for GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL in GSITESTClientPanel.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_TEXT_ENTER event handler for GSI_ID_CLIENT_TEST_NB_COMBO_BOX_ALL in GSITESTClientPanel.
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for GSI_ID_CLIENT_TEST_KD_BUTTON1
 */

void GSITestClientPanel::OnGsiIdClientTestKdButton1Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for GSI_ID_CLIENT_TEST_KD_BUTTON1 in GSITESTClientPanel.
    // Before editing this code, remove the block markers.
    GSIClientTestFrame* window = new GSIClientTestFrame(this, GSI_ID_CLIENT_TESTFRAME, _("GSICLientTestFrame"));
    window->Show(true);
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for GSI_ID_CLIENT_TEST_KD_BUTTON1 in GSITESTClientPanel.
}

/*!
 * Should we show tooltips?
 */

bool GSITestClientPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap GSITestClientPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin GSITestClientPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end GSITestClientPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon GSITestClientPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin GSITestClientPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end GSITestClientPanel icon retrieval
}
