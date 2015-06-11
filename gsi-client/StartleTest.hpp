
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
#ifndef STLTEST_HPP_INCLUDED
#define STLTEST_HPP_INCLUDED

#include "../gsi-server/include/startledefs.h"
#include "gsiclientnbpanel.h"
#include "GSIClientTestFrame.h"
#include "gsitest.hpp"

class StartleLUT;

enum
{
STL_ID_MENU_ITEM_VOL_FREQ=GSIFirstUserId
};

class STLClientTestFrame:public GSIClientTestFrame
{
public:
    STLClientTestFrame();
    STLClientTestFrame( wxWindow* parent, wxWindowID id = SYMBOL_GSICLIENTTESTFRAME_IDNAME, const wxString& caption = SYMBOL_GSICLIENTTESTFRAME_TITLE, const wxPoint& pos = SYMBOL_GSICLIENTTESTFRAME_POSITION, const wxSize& size = SYMBOL_GSICLIENTTESTFRAME_SIZE, long style = SYMBOL_GSICLIENTTESTFRAME_STYLE );

    virtual		~STLClientTestFrame();

    void		OnTest1(wxCommandEvent& event);  //overide GSITest test1 function
    void		OnTest2(wxCommandEvent& event);  //overide GSITest test2 function

    void		OnDebugSocket(wxCommandEvent& event);                               //override menu
	void		OnSetVolFreq(wxCommandEvent& event);
//Combo box events for Startle SetVolumeFrequency dialog
	void        OnSetVolFreqCB ( wxCommandEvent& event);
	void		OnStlIdClientTestSineVolCbUpdated( wxCommandEvent& event);
	void		OnGsiIdClientTestMenuOpenNodeClick(wxCommandEvent& event);	//handle this GSI event here
#if 0
	void		OnStlIdClientTestSineVolCbEnter ( wxCommandEvent& event);
	void		OnStlIdClientTestWNVolCbEnter( wxCommandEvent& event);
	void		OnStlIdClientTestSineFrequencyCbEnter( wxCommandEvent& event);
#endif
	errorType	OpenNode(GSIClientNBPanelData  & p);
	errorType	OpenDevice(GSIDevHandle h);
	errorType	OpenDevice(const wxString & dev_str);
	errorType	CloseDevice(GSIDevHandle h_dev);
	//errorType	AddNode(GSINodeHandle node);
	errorType	SetFrequency(wxUint16 f,STLQueuedFlag q=STL_IMMEDIATE,wxUint16 at_tick=0);

	errorType	SetAmplitudeSPL(wxInt16 dB,STLQueuedFlag q=STL_IMMEDIATE,wxUint16 at_tick=0);
	errorType	SetNoiseAmplitudeSPL(wxInt16 dB,STLQueuedFlag q=STL_IMMEDIATE,wxUint16 at_tick=0);

	errorType	Reset(STLQueuedFlag q=STL_IMMEDIATE,wxUint16 at_tick=0);	//currently always immediate
	errorType	SetChannel(wxInt16 chan,STLQueuedFlag q=STL_IMMEDIATE,wxUint16 at_tick=0);
	errorType	Acquire(wxInt16 num_data_points,STLQueuedFlag q=STL_IMMEDIATE,wxUint16 at_tick=0);
	errorType	Execute();		//always IMMEDIATE
	errorType	WaitForCompletion();

	errorType	SetAmplitudeLUTValue(wxInt16 index,wxInt16 value);
	errorType	SetNoiseAmplitudeLUTValue(wxInt16 index,wxInt16 value);



	void		TestAllFunctions(int dev_num,STLQueuedFlag q_flag=STL_IMMEDIATE);
	void		LiveTest();
	errorType	WriteADCDataToDisk(const wxString & fname,wxInt16 num_ints,wxInt16 *const buffer);
    errorType SendRecord(wxInt16 type, wxInt16 flag=STL_IMMEDIATE,wxInt16 param0=0, wxInt16 param1=0, wxInt16 param2=0, wxInt16 param3=0, wxInt16 param4=0, wxInt16 param5=0);
    void		DisplayData();       //display any data received in text control
    void		DisplayReturnCode();
//Access functions
	wxSocketBase &	GetSock();
//use these to talk to the remote node
							GSINodeHandle GetNodeHandle() const {return m_PanelData->GetNodeHandle();}
			void			SetNodeHandle(GSINodeHandle h) {m_PanelData->SetNodeHandle(h);}
			GSIClientNBPanelData & GetPanel() {return *m_PanelData;}

			StartleLUT		&				GetdBLUT() {return *m_dBLUT;}
			StartleLUT		&				GetdBNoiseLUT(){return *m_dBNoiseLUT;}

private:
    wxInt16 m_recordArray[STLRecordArraySize];
    int     m_comPort;
	wxMenu	*m_MenuDebugSocket;
	wxMenu	*m_MenuStartle;

	//All controls, including device, node (the hardware) are accessed through
	//a notebook panel. The panel only holds valid data once a node has been
	//opened. All access should be though an access function to ensure we don't
	//try to acces an incomplete object
	//
	GSIClientNBPanelData *m_PanelData;				//access node etc through here

	StartleLUT		* m_dBLUT;
	StartleLUT		* m_dBNoiseLUT;

  DECLARE_EVENT_TABLE()
};



const  char MenuDebugStartleText[]=	"Debug Startle";
const char MenuStartleText[] =			"Startle";
#endif
