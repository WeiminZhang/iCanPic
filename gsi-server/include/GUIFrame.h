///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GUIFrame__
#define __GUIFrame__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

#define idMenuQuit 1000
#define idMenuAbout 1001

///////////////////////////////////////////////////////////////////////////////
/// Class GUIFrame
///////////////////////////////////////////////////////////////////////////////
class GUIFrame : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* mbar;
		wxMenu* fileMenu;
		wxMenu* helpMenu;
		wxStatusBar* statusBar;
		wxTextCtrl* m_textCtrlAddress;
		wxCheckBox* m_checkBoxAddress;
		wxTextCtrl* m_textCtrlDesc;
		wxCheckBox* m_checkBoxDescription;
		wxListBox* m_listBoxProducts;
		wxStdDialogButtonSizer* m_OKCancel;
		wxButton* m_OKCancelOK;
		wxButton* m_OKCancelCancel;
		wxButton* m_btnClear;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_textCtrlAddressOnText( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_textCtrlDescOnText( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_listBoxProductsOnKeyUp( wxKeyEvent& event ) { event.Skip(); }
		virtual void m_listBoxProductsOnListBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_listBoxProductsOnListBoxDClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_OKCancelOnCancelButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_OKCancelOnOKButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_btnClearOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		GUIFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("wxWidgets Application Template"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 604,549 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~GUIFrame();
	
};

#endif //__GUIFrame__
