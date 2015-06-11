///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "GUIFrame.h"

///////////////////////////////////////////////////////////////////////////

GUIFrame::GUIFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	mbar = new wxMenuBar( 0 );
	fileMenu = new wxMenu();
	wxMenuItem* menuFileQuit;
	menuFileQuit = new wxMenuItem( fileMenu, idMenuQuit, wxString( wxT("&Quit") ) + wxT('\t') + wxT("Alt+F4"), wxT("Quit the application"), wxITEM_NORMAL );
	fileMenu->Append( menuFileQuit );

	mbar->Append( fileMenu, wxT("&File") );

	helpMenu = new wxMenu();
	wxMenuItem* menuHelpAbout;
	menuHelpAbout = new wxMenuItem( helpMenu, idMenuAbout, wxString( wxT("&About") ) + wxT('\t') + wxT("F1"), wxT("Show info about this application"), wxITEM_NORMAL );
	helpMenu->Append( menuHelpAbout );

	mbar->Append( helpMenu, wxT("&Help") );

	this->SetMenuBar( mbar );

	statusBar = this->CreateStatusBar( 2, wxST_SIZEGRIP, wxID_ANY );
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Address") ), wxHORIZONTAL );

	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxVERTICAL );

	m_textCtrlAddress = new wxTextCtrl( this, wxID_ANY, wxT("Glenn Self\n35 Eclipse way\nBeckenham \nWA\n6107"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_MULTILINE );
	m_textCtrlAddress->SetMinSize( wxSize( 300,-1 ) );

	bSizer41->Add( m_textCtrlAddress, 0, wxALL, 5 );

	m_checkBoxAddress = new wxCheckBox( this, wxID_ANY, wxT("Append to file"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer41->Add( m_checkBoxAddress, 0, wxALL, 5 );

	sbSizer3->Add( bSizer41, 1, wxALIGN_TOP, 5 );

	bSizer2->Add( sbSizer3, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Description") ), wxHORIZONTAL );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	m_textCtrlDesc = new wxTextCtrl( this, wxID_ANY, wxT("Holden RA Rodeo Rear light Tail light 06-08 LHS"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_MULTILINE );
	m_textCtrlDesc->SetMinSize( wxSize( 300,40 ) );

	bSizer5->Add( m_textCtrlDesc, 0, wxALL, 5 );

	m_checkBoxDescription = new wxCheckBox( this, wxID_ANY, wxT("Append to file"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_checkBoxDescription, 0, wxALL, 5 );

	m_listBoxProducts = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( 480,120 ), 0, NULL, wxLB_NEEDED_SB );
	m_listBoxProducts->SetMaxSize( wxSize( -1,120 ) );

	bSizer5->Add( m_listBoxProducts, 0, wxALIGN_TOP, 5 );

	sbSizer1->Add( bSizer5, 1, wxEXPAND, 5 );

	bSizer2->Add( sbSizer1, 1, wxEXPAND, 5 );

	m_OKCancel = new wxStdDialogButtonSizer();
	m_OKCancelOK = new wxButton( this, wxID_OK );
	m_OKCancel->AddButton( m_OKCancelOK );
	m_OKCancelCancel = new wxButton( this, wxID_CANCEL );
	m_OKCancel->AddButton( m_OKCancelCancel );
	m_OKCancel->Realize();
	bSizer2->Add( m_OKCancel, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	bSizer4->Add( bSizer2, 1, wxEXPAND, 5 );

	bSizer3->Add( bSizer4, 1, wxEXPAND, 5 );

	m_btnClear = new wxButton( this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( m_btnClear, 0, wxALIGN_CENTER|wxALL, 5 );

	this->SetSizer( bSizer3 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIFrame::OnClose ) );
	this->Connect( menuFileQuit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( GUIFrame::OnQuit ) );
	this->Connect( menuHelpAbout->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( GUIFrame::OnAbout ) );
	m_textCtrlAddress->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIFrame::m_textCtrlAddressOnText ), NULL, this );
	m_textCtrlDesc->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIFrame::m_textCtrlDescOnText ), NULL, this );
	m_listBoxProducts->Connect( wxEVT_KEY_UP, wxKeyEventHandler( GUIFrame::m_listBoxProductsOnKeyUp ), NULL, this );
	m_listBoxProducts->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( GUIFrame::m_listBoxProductsOnListBox ), NULL, this );
	m_listBoxProducts->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( GUIFrame::m_listBoxProductsOnListBoxDClick ), NULL, this );
	m_OKCancelCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIFrame::m_OKCancelOnCancelButtonClick ), NULL, this );
	m_OKCancelOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIFrame::m_OKCancelOnOKButtonClick ), NULL, this );
	m_btnClear->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIFrame::m_btnClearOnButtonClick ), NULL, this );
}

GUIFrame::~GUIFrame()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GUIFrame::OnClose ) );
	this->Disconnect( idMenuQuit, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( GUIFrame::OnQuit ) );
	this->Disconnect( idMenuAbout, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( GUIFrame::OnAbout ) );
	m_textCtrlAddress->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIFrame::m_textCtrlAddressOnText ), NULL, this );
	m_textCtrlDesc->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GUIFrame::m_textCtrlDescOnText ), NULL, this );
	m_listBoxProducts->Disconnect( wxEVT_KEY_UP, wxKeyEventHandler( GUIFrame::m_listBoxProductsOnKeyUp ), NULL, this );
	m_listBoxProducts->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( GUIFrame::m_listBoxProductsOnListBox ), NULL, this );
	m_listBoxProducts->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( GUIFrame::m_listBoxProductsOnListBoxDClick ), NULL, this );
	m_OKCancelCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIFrame::m_OKCancelOnCancelButtonClick ), NULL, this );
	m_OKCancelOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIFrame::m_OKCancelOnOKButtonClick ), NULL, this );
	m_btnClear->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GUIFrame::m_btnClearOnButtonClick ), NULL, this );

}
