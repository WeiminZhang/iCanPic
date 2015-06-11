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

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma implementation
    #pragma interface
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "wx/socket.h"
#include "wx/url.h"
#include "wx/wfstream.h"
#include "wx/numdlg.h"
#include "wx/bookctrl.h"
#include "wx/ffile.h"

//#include "gsi-common/include/gsi.hpp"
#ifdef STARTLE_SERVER       //see gsidefs.h
#warning  ("Compiling Startle box code")
#include "../include/startledefs.h"
#include "gsiclientnbpanel.h"

#include "gsi-common/include/socket.hpp"
#include "client.hpp"
#include "lcdClient.hpp"


#include "../socket/startle.hpp"

#include <gsExcept.hpp>               //exception handling support. Requires
#include "gsitest.hpp"
#include "GSIClientTestFrame.h"
#include "../socket/Startledefs.h"
#include "StartleTest.hpp"
#include "STLClientCommands.hpp"
#include "STLVolFreq.h"
#include "setdeviceDialog.h"
#include "../socket/helper.hpp"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif


BEGIN_EVENT_TABLE(STLClientTestFrame, GSIClientTestFrame)
    EVT_MENU(CLIENT_DEBUG_USER1,    STLClientTestFrame::OnDebugSocket)
	EVT_MENU(CLIENT_DEBUG_USER2,    STLClientTestFrame::OnDebugSocket)
	EVT_MENU(STL_ID_MENU_ITEM_VOL_FREQ,  STLClientTestFrame::OnSetVolFreq)

	EVT_MENU(GSI_ID_CLIENT_TEST_MENU_OPEN_NODE, STLClientTestFrame::OnGsiIdClientTestMenuOpenNodeClick)
END_EVENT_TABLE()


STLClientTestFrame::STLClientTestFrame( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size , long style)
:GSIClientTestFrame(parent,id,caption,pos, size,style )
{
	wxMenuBar * mb;

	m_MenuStartle = DEBUG_NEW wxMenu();
	mb=GetMenuBar();

	mb->Append(m_MenuStartle,_("Startle"));
    m_MenuStartle->Append(STL_ID_MENU_ITEM_VOL_FREQ, _("&Set Volume+Frequency"), _("Vol Freq"));


	m_MenuDebugSocket = DEBUG_NEW wxMenu();
	mb=GetMenuBar();

	mb->Append(m_MenuDebugSocket,_("Debug Startle"));
    m_MenuDebugSocket->Append(CLIENT_DEBUG_USER1, _("Test &all functions"), _("Test 1"));


    m_MenuDebugSocket->Append(CLIENT_DEBUG_USER2, _("&Freq scan "), _("Freq scan"));




//the following code is req'd as we have a circular dependency. This is because
//we are using a single panel with a single node so can use the short cut of
//GSI_NODE_THIS to retrieve the required panel. Must also ensure that the node
//is set within the panel
	m_PanelData= GetPanelData(GSI_NODE_THIS);
	SetNodeHandle(GSINotAValidNodeHandle);
	SetDeviceHandle(GSINotAValidDevHandle);

    m_dBLUT=DEBUG_NEW LUT(STLMaxLUTIndex+1,STLMaxAmplitudeDB);
    m_dBNoiseLUT=DEBUG_NEW LUT(STLMaxLUTIndex+1,STLMaxNoiseAmplitudeDB);

}


STLClientTestFrame::~STLClientTestFrame()
{
}

/******************************************************************

    void STLClientTestFrame::OnTest1(wxCommandEvent& event)

*******************************************************************/
void STLClientTestFrame::OnTest1(wxCommandEvent& event)
{
    event;
    //SetBusyFlag(true);
    //UpdateStatusBar();
/*Test them all*/
    try
    {

    }
    catch(gsException &exc)
    {
        wxString s=exc.GetUserString();
    }

    //SetChannel(1,flag,50);             //channel,flag, at_tick
    //SetAmplitude(100,flag,100);        //amplitude,flag,at_tick

    //SetBusyFlag(false);
    //SockClientStatusBar()->Update();

}

/******************************************************************

    void STLClientTestFrame::OnTest2(wxCommandEvent& event)

*******************************************************************/
void STLClientTestFrame::OnTest2(wxCommandEvent& event)
{
    event;
}


/*!
	errorType STLClientTestFrame::OpenDevice(const wxString &dev_str)
*/
errorType STLClientTestFrame::OpenDevice(const wxString &dev_str)
{
int rc;
	STLClientCommandOpenDeviceXXX Cmd(m_PanelData->GetSocketClient(),dev_str,STL_IMMEDIATE,0,GetPanel().GetStatusText());
	Cmd.SetStatusText(GetPanel().GetStatusText());
	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		if(Cmd.GetReturnNumInts()==1)

		{	//returns device handle if device opened but node open failed
			eStr.Printf(_("Error Open Device: Failed to open node (Startle box not on?)"));
		}
		else
			eStr.Printf(_("Error Open Device: %s"),Cmd.GetClientErrorString());
		wxMessageBox(_(eStr),_("Error:"),wxOK | wxICON_EXCLAMATION, this);
		if(rc==GSISocketReturnData::GSI_ALREADY_OPEN)
			return errAlreadyOpen;
		return errBadDevice;
	}
	m_PanelData->SetDeviceName(Cmd.GetDeviceString());
	m_PanelData->SetDeviceHandle(Cmd.GetDeviceHandle());
	wxInt16* rd=Cmd.GetReturnData();
	m_PanelData->SetNodeHandle(rd[1]);
//node is in returnedData[1], use it!
	UpdateNBPage(*m_PanelData);
	return errNone;


}

/*!
	errorType STLClientTestFrame::OpenDevice(wxInt16 com_port)
*/
errorType STLClientTestFrame::OpenDevice(GSIDevHandle h)
{
GSIDevices d;

	return OpenDevice(d.GetDeviceString(h));

}


/*!
	errorType STLClientTestFrame::OpenDevice(wxInt16 com_port)
*/
errorType STLClientTestFrame::CloseDevice(GSIDevHandle h_dev)
{
int rc;
	STLClientCommandCloseDeviceXXX Cmd(m_PanelData->GetSocketClient(),
		h_dev,
		STL_IMMEDIATE,
		0,
		m_PanelData->GetStatusTextCtrl());
	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Close Device: %s"),Cmd.GetErrorString());
		wxMessageBox(_(eStr),_("Error:"),wxOK | wxICON_EXCLAMATION, this);
		return errFileClose;
	}
	m_PanelData->SetDeviceName("");
	m_PanelData->SetDeviceHandle(GSINotAValidDevHandle);
	UpdateNBPage(*m_PanelData);
	return errNone;
}


/*!
errorType STLClientTestFrame::SetFrequency(wxUint16 f)
*/
errorType STLClientTestFrame::SetFrequency(wxUint16 f,STLQueuedFlag q,wxUint16 at_tick)
{
	int rc;

	STLClientCommandSetFrequencyXXX Cmd(m_PanelData->GetSocketClient(), //socket
		f,		//frequency
		q,	//queued flag
		at_tick,		//at_tick
		m_PanelData->GetStatusTextCtrl()
		 );
	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Set Frequency: %s"),Cmd.GetErrorString());

		//xxx server should return No Device or other error rather than "fail"
		wxMessageBox(_(eStr),_("Error Set Frequency:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}

/*!
errorType STLClientTestFrame::SetAmplitudeSPL(wxInt16 f)
*/
errorType STLClientTestFrame::SetAmplitudeSPL(wxInt16 dB,STLQueuedFlag q,wxUint16 at_tick)
{
	int rc;
//STLCommandSetAmplitudeSPL(LUT & lut,wxInt16 dB=0,wxInt16 queued=STL_IMMEDIATE,wxUint16 at_tick=0, wxInt16 node=GSI_NODE_THIS);
	//STLCommandSetAmplitudeSPL scsasCmd(dB);

    LUT lut(STLMaxLUTIndex+1,STLMaxAmplitudeDB); //has to be big enough for largest index else it will THROW!

	STLClientCommandSetAmplitudeSPLXXX Cmd(m_PanelData->GetSocketClient(), //socket
		dB,			//dB
		lut,
		q,			//queued flag
		at_tick,	//at_tick
		m_PanelData->GetStatusTextCtrl()
		 ); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Set Amplitude: %s"),Cmd.GetClientErrorString());

		wxMessageBox(_(eStr),_("Error Set AmplitudeSPL:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}

/*!
errorType STLClientTestFrame::SetNoiseAmplitudeSPL(wxInt16 f)
*/
errorType STLClientTestFrame::SetNoiseAmplitudeSPL(wxInt16 dB,STLQueuedFlag q,wxUint16 at_tick)
{
	int rc;
	LUT lut(STLMaxLUTIndex+1,STLMaxNoiseAmplitudeDB); //dummy LUT must be large enough for largest index or will THROW
	//STLCommandSetNoiseAmplitudeSPL scsnasCmd(dB);
	STLClientCommandSetNoiseAmplitudeSPLXXX Cmd(m_PanelData->GetSocketClient(), //socket
		dB,			//dB
		lut,
		q,			//queued flag
		at_tick,	//at_tick
		m_PanelData->GetStatusTextCtrl()
		); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Set Noise Amplitude: %s"),Cmd.GetErrorString());

		wxMessageBox(_(eStr),_("Error Set Noise AmplitudeSPL:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}


/*!
errorType STLClientTestFrame::Reset()
*/
errorType STLClientTestFrame::Reset(STLQueuedFlag q,wxUint16 at_tick)	//currently always immediate
{
	int rc;
	STLClientCommandResetXXX Cmd(m_PanelData->GetSocketClient(), //socket
		q,			//queued flag
		at_tick,	//at_tick
		m_PanelData->GetStatusTextCtrl()
		 ); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Reset: %s"),Cmd.GetErrorString());

		wxMessageBox(_(eStr),_("Error Reset:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}

/*!
errorType STLClientTestFrame::SetChannelSPL(wxInt16 chan,STLQueuedFlag q,wxUint16 at_tick)
*/
errorType STLClientTestFrame::SetChannel(wxInt16 chan,STLQueuedFlag q,wxUint16 at_tick)

{
	int rc;
	//STLCommandSetChannel scCmd(chan);
	STLClientCommandSetChannelXXX Cmd(m_PanelData->GetSocketClient(), //socket
		chan,			//dB
		q,			//queued flag
		at_tick,	//at_tick
		m_PanelData->GetStatusTextCtrl()
		 ); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Set Channel: %s"),Cmd.GetErrorString());

		wxMessageBox(_(eStr),_("Error Set Channel:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}



/*!
errorType STLClientTestFrame::Acquire(wxInt16 num_data_points,STLQueuedFlag q,wxUint16 at_tick)
*/
errorType STLClientTestFrame::Acquire(wxInt16 num_data_points,STLQueuedFlag q,wxUint16 at_tick)

{
	int rc;
	//STLCommandAcquire ACmd(num_data_points);
	STLClientCommandAcquireXXX Cmd(m_PanelData->GetSocketClient(), //socket
		num_data_points,			//dB
		q,			//queued flag
		at_tick,	//at_tick
		m_PanelData->GetStatusTextCtrl()
		 ); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Acquire Channel: %s"),Cmd.GetErrorString());

		wxMessageBox(_(eStr),_("Error Acquire:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	else
	{	//no error, so write to disk
		if(q==STL_IMMEDIATE)
		{

			WriteADCDataToDisk("STLADC.txt",Cmd.GetReturnNumInts(),Cmd.GetReturnData());
		}
	}
	return errNone;
}


/*!
errorType STLClientTestFrame::Execute()
*/
errorType STLClientTestFrame::Execute()

{
	int rc;
	//STLCommandExecute ECmd;
	STLClientCommandExecuteXXX Cmd(m_PanelData->GetSocketClient(), //socket
		STL_IMMEDIATE,			//queued flag. flag ignored, always immediate
		0,						//at_tick. Ignored, always 0
		m_PanelData->GetStatusTextCtrl()
		 ); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Execute: %s"),Cmd.GetErrorString());

		wxMessageBox(_(eStr),_("Error Execute:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	else
	{
		WriteADCDataToDisk("STLADC.txt",Cmd.GetReturnNumInts(),Cmd.GetReturnData());
	}
	return errNone;
}

/*!
errorType STLClientTestFrame::WaitForCompletion()
*/
errorType STLClientTestFrame::WaitForCompletion()
{
	int rc;
	//STLCommandWaitForCompletion wCmd;


	STLClientCommandWaitForCompletionXXX Cmd(m_PanelData->GetSocketClient(), //socket
		STL_IMMEDIATE,	//queued flag always IMMEDIATE
		0,  				//at_tick
		m_PanelData->GetStatusTextCtrl()
		 );
	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Set Frequency: %s"),Cmd.GetErrorString());

		//xxx server should return No Device or other error rather than "fail"
		wxMessageBox(_(eStr),_("Error Set Frequency:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}


/**********************************************************************

	void STLClientTestFrame::OnSetVolFreq(wxCommandEvent& event)

***********************************************************************/

void STLClientTestFrame::OnSetVolFreq(wxCommandEvent& event)
{
static wxString PrevVol="0";		//CB selected
static wxString PrevWNVol="0";		//CB selected
static wxString PrevFreq="5000";	//CB selected


	event;
	if (IsNodeOpen()==false)
	{
		GSIClientNBPanelData  & p=GetPanel();
		if(OpenNode(p))
			return;
		STLClientCommandResetXXX(GetSock(),STL_IMMEDIATE,0,GetPanel().GetStatusText());
	}
	STLSetVolFreqDialog* dlg = DEBUG_NEW STLSetVolFreqDialog(this, STL_ID_VOL_FREQ_DIALOG, _("Open Set Vol Freq dialog"));
//Connect our event handler to the CB's
	dlg->Connect(STL_ID_CLIENT_TEST_SINE_VOL_CB,
					wxEVT_COMMAND_TEXT_ENTER,
					wxCommandEventHandler(STLClientTestFrame::OnSetVolFreqCB),
					NULL,
					this
					);
	dlg->Connect(STL_ID_CLIENT_TEST_WN_VOL_CB,
					wxEVT_COMMAND_TEXT_ENTER,
					wxCommandEventHandler(STLClientTestFrame::OnSetVolFreqCB),
					NULL,
					this
					);
	dlg->Connect(STL_ID_CLIENT_TEST_SINE_FREQ_CB,
					wxEVT_COMMAND_TEXT_ENTER,
					wxCommandEventHandler(STLClientTestFrame::OnSetVolFreqCB),
					NULL,
					this
					);
#if 1
//Set values to previously set values
	dlg->GetSineDBCB()->SetValue(PrevVol);
	dlg->GetNoiseDBCB()->SetValue(PrevWNVol);
	dlg->GetSineFrequencyDBCB()->SetValue(PrevFreq);
#endif
    int returnValue = dlg->ShowModal();

		wxString s=PrevVol;
		s=PrevWNVol;
		s=PrevFreq;

    if(returnValue == wxID_OK)
    {
#if 1
		//update previous values
		PrevVol=dlg->GetSineDBCB()->GetValue();
		PrevWNVol=dlg->GetNoiseDBCB()->GetValue();
		PrevFreq=dlg->GetSineFrequencyDBCB()->GetValue();
#endif
    }
	else
	{
#if 0
		//reset to original
		dlg->GetSineDBCB()->SetSelection(PrevVol);
		dlg->GetNoiseDBCB()->SetSelection(PrevWNVol);
		dlg->GetSineFrequencyDBCB()->SetSelection(PrevFreq);
#endif
	}
	long data=0;
	PrevVol.ToLong(&data);
	STLClientCommandSetAmplitudeSPLXXX(GetSock(),data,GetdBLUT(),STL_IMMEDIATE,0,GetPanel().GetStatusText());

	PrevWNVol.ToLong(&data);
	STLClientCommandSetNoiseAmplitudeSPLXXX(GetSock(),data,GetdBNoiseLUT(),STL_IMMEDIATE,0,GetPanel().GetStatusText());

	PrevFreq.ToLong(&data);
	STLClientCommandSetFrequencyXXX (GetSock(),data,STL_IMMEDIATE,0,GetPanel().GetStatusText());

    dlg->Destroy();
}
/**********************************************************************

	void STLClientTestFrame::OnDebugSocket(wxCommandEvent& event)

***********************************************************************/

void STLClientTestFrame::OnDebugSocket(wxCommandEvent& event)
{
	errorType rv=errNone;
    int id=event.GetId();
	//STLFrequencyParam f=1000;   //dummy frequency
	int dev=6;		//comport number
	try
	{
		switch (id)
		{
		case CLIENT_DEBUG_USER1:  //Test all functions
			TestAllFunctions(dev,STL_QUEUED);
			break;
		case CLIENT_DEBUG_USER2:
			{
				wxUint16 i=0;
				rv=OpenDevice(dev);
				if(rv)
				{
					if(rv != errAlreadyOpen)		//ignore this error
						return;
				}

				rv=Reset(STL_IMMEDIATE);
				if(rv)
					return;

				rv=SetFrequency(5000,STL_IMMEDIATE);

				for(i=0;i<120;i++)
				{
					rv=SetAmplitudeSPL(i,STL_IMMEDIATE);
				}

				rv=SetAmplitudeSPL(120,STL_IMMEDIATE);
				if(rv)
					return;

	//Sweep Frequency
				for(i=0;i<50000;i+=100)
				{
					rv=SetFrequency(i,STL_IMMEDIATE);
					if(rv)
						break;
				}
			}
			break;
		case CLIENT_DEBUG_USER3:
			break;
		case CLIENT_DEBUG_USER4:
			break;
		case CLIENT_DEBUG_USER5:
			break;
		default:
			event.Skip();
			break;
		}
	}
	catch(gsException &exc)
	{
		wxString s;

		s.Printf( "%s\n%s\n\n",exc.GetString(),exc.GetUserString());
		m_PanelData->GetStatusTextCtrl()->AppendText(s);
	}
	UpdateNBPage(*m_PanelData);


    return;
}


/*!
void STLClientTestFrame::TestAllFunctions(int dev_num)
*/

void STLClientTestFrame::TestAllFunctions(int dev_num,STLQueuedFlag q_flag)
{
errorType rv;
wxString s;
wxString s1="";
float dB=70;
float dBNoise=80;
wxUint16 f=5000;
//wxInt16 lutIndex=5;
//wxInt16 lutValue=10;
wxString comPort="COM6";
int i;
wxUint16 numDataPoints=1000;
GSINodeHandle node=GSI_NODE_THIS;

	dev_num;
	GSIClientNBPanelData  & p=GetPanel();

	wxTextCtrl &t=*p.GetStatusTextCtrl();

	t.AppendText("Testing all startle functions\n");
	if(q_flag==STL_IMMEDIATE)
		s.Printf("Functions are IMMEDIATE\n");
	else
		s.Printf("Functions are QUEUED (Execute function begins)\n");


//Open socket
    if(! p.GetSocketClient().IsConnected())
    {
        errorType rv=OpenSocketSession();
        if(rv)
            return;
    }


//--------------------------------------------------

//-----------------Open Device (will open node as well)----------------------------
	node;
	t.AppendText("Open Device (always IMMEDIATE)\n");
	rv=OpenDevice(comPort);	//always immediate
	if(rv)
		return;
//--------------------------------------------------

//-----------------Reset----------------------------
	t.AppendText("Reset (always IMMEDIATE)\n");
	rv=Reset(STL_IMMEDIATE);	//always immediate
	if(rv)
		return;

//-------------------------------------------------

//-------------SetAmplitudeLUTVal --------------------
#if 0
	s.Printf(_("SetAmplitudeLUTValue index %i to value %i (uses LUT)\n"),lutIndex,lutValue);
	t.AppendText(s);
	rv=SetAmplitudeLUTValue(lutIndex,lutValue);
	if(rv)
		return;
#endif
//----------------------------------------------------

#if 0
//-------------SetNoiseAmplitudeLUTVal --------------------
	s.Printf(_("SetNoisAmplitudeLUTValue index %i to value %i (uses LUT)\n"),lutIndex,lutValue);
	t.AppendText(s);
	rv=SetNoiseAmplitudeLUTValue(lutIndex,lutValue);
	if(rv)
		return;
#endif
//----------------------------------------------------

//-------------SetAmplitudeSPL --------------------
	s.Printf(_("SetAmplitudeSPL to %i dB (uses LUT)\n"),dB);
	t.AppendText(s);
	rv=SetAmplitudeSPL(dB,q_flag);
	if(rv)
		return;

//----------------------------------------------------

//-------------SetNoiseAmplitudeSPL --------------------
	s.Printf(_("SetNoiseAmplitudeSPL to %i dB (uses LUT)\n"),dBNoise);
	t.AppendText(s);
	rv=SetNoiseAmplitudeSPL(dBNoise,q_flag);
	if(rv)
		return;

//------------------SetFrequency----------------------
	s.Printf(_("SetFrequency to %i Hz\n"),f);
	t.AppendText(s);
	rv=SetFrequency(f,q_flag);
	if(rv)
		return;

//------------------------------------------------

//-----SetChannel---------------------------------
	for(i=0;i<STLMaxChan;i++)
	{
		s.Printf(_("Set chan %i "),i);
		t.AppendText(s);

		rv=SetChannel(i,q_flag);
		if(rv)
			return;
	}
	rv=SetChannel(0,q_flag);		//for testing purposes always use chan 0
	t.AppendText("\n");
//-------------------------------------------

//---------Acquire---------------------------
	s.Printf(_("Acquire %i data points\n"),numDataPoints);
	t.AppendText(s);

	rv=Acquire(1000,q_flag);
	if(rv)
		return;
//--------------------------------------------------

//----------Execute-------------------------------

	if(q_flag==STL_QUEUED)
	{
		s.Printf(_("Execute\n"));
		t.AppendText(s);

		rv=Execute();
		if(rv)
			return;
	}
//---------------------------------------------------------

//------- WaitForCompletion ----------------------------------
	t.AppendText(_("WaitForCompletion\n"));
	rv=WaitForCompletion();
	if(rv)
		return;
//--------------------------------------------------

//-----------------CloseDevice	---------------------------
	s.Printf(_("Close device (handle=%i)\n"),GetPanel().GetDeviceHandle());
	t.AppendText(s);


	rv=CloseDevice(GetPanel().GetDeviceHandle());
	if(rv)
		return;
//-----------------------------------------------------
}

errorType	STLClientTestFrame::WriteADCDataToDisk(const wxString & fname,wxInt16 num_ints,wxInt16 *const buffer)
{
wxString s;
int i;

	wxFFile ofile(fname,"wt");
	if(ofile.IsOpened())
	{
		for(i=0;i<num_ints;i++)
		{
			s.Printf("%i ",buffer[i]);
			ofile.Write(s);
		}
	}
	else
		return errFail;

	return errNone;
}



/*!
errorType STLClientTestFrame::SetAmplitudeLUTValue(lutIndex,lutValue,q_flag)
*/
errorType STLClientTestFrame::SetAmplitudeLUTValue(wxInt16 index,wxInt16 value)
{
	int rc;
	//STLCommandSetAmplitudeLUTValue LCmd(index,value);

	STLClientCommandSetAmplitudeLUTValueXXX Cmd(m_PanelData->GetSocketClient(), //socket
		index,value,m_PanelData->GetStatusTextCtrl()); //output status text to here

	rc=Cmd.GetReturnCode();
	if(rc)
	{
		wxString eStr;
		eStr.Printf(_("Error Set Amplitude: %s"),Cmd.GetErrorString());

		wxMessageBox(_(eStr),_("Error Set AmplitudeSPL:"),wxOK | wxICON_EXCLAMATION, this);
		return errGSIFail;
	}
	return errNone;
}


/*!
void STLClientTestFrame::OnSetVolFreqCB ( wxCommandEvent& event)
A combo box within the SetVolFreq dialog has had return pressed
while in the edit field.
We must send the command to the server
*/
void STLClientTestFrame::OnSetVolFreqCB ( wxCommandEvent& event)
{
//Currently arrive here with this * referring to STLVolFreq dialog context,
//hence utter madness when we reference our data
//Fixed by using Connect(), as long as EventSink parameter is given this rather than NULL

//dummy LUT variable
LUT lut(STLMaxLUTIndex+1,STLMaxAmplitudeDB); //has to be big enough for largest index else it will THROW!

//Have to use the GetString() function rather than GetInt()
//because the stored integer has zero's stripped from it.
//So a frequency of 15000 is returned as 15 (as is a frequency of 15)
	wxString s=event.GetString();
	long l;
	s.ToLong(&l);
	int data=static_cast <int>(l);


	switch (event.GetId())
	{
	case STL_ID_CLIENT_TEST_SINE_VOL_CB:
		{
			STLClientCommandSetAmplitudeSPLXXX Cmd(GetSock(),data,lut,STL_IMMEDIATE,0,GetPanel().GetStatusText());
		}
		break;

	case STL_ID_CLIENT_TEST_WN_VOL_CB:
		{
			STLClientCommandSetNoiseAmplitudeSPLXXX Cmd(GetSock(),data,lut,STL_IMMEDIATE,0,GetPanel().GetStatusText());
		}
		break;
	case STL_ID_CLIENT_TEST_SINE_FREQ_CB:
		{
			STLClientCommandSetFrequencyXXX Cmd(GetSock(),data,STL_IMMEDIATE,0,GetPanel().GetStatusText());
		}
		break;

	default:
		{
			int i=0;
			i++;
		}
		break;
	}
}


void STLClientTestFrame::OnGsiIdClientTestMenuOpenNodeClick (wxCommandEvent& event)
{
	event;
	GSIClientNBPanelData  & p=GetPanel();
	OpenNode(p);
}

/*!
STLClientTestFrame::OpenNode(GSIClientNBPanelData  & p)
*/
errorType STLClientTestFrame::OpenNode(GSIClientNBPanelData  & p)
{
	errorType rv=errNone;
    wxString DevS;
	GSIDevices Devices;
	int deviceId;

    if(! p.GetSocketClient().IsConnected())
    {
        errorType rv=OpenSocketSession();
        if(rv)
            return rv;
    }

	SetDeviceDialog* window = DEBUG_NEW SetDeviceDialog(this, GSI_ID_CLIENT_TEST_SETDEVICE_DIALOG, _("Set device"));
    int returnValue = window->ShowModal();
    if(returnValue == wxID_OK)
    {
        deviceId=window->m_Choice->GetCurrentSelection();
        DevS=window->m_Choice->GetString(deviceId);
	}

	if( ! IsDeviceOpen(DevS))
	{
		rv=OpenDevice(DevS);
	}
	return rv;
}
/*!
wxSocketBase &	STLClientTestFrame::GetSock()const
*/
wxSocketBase &	STLClientTestFrame::GetSock()
{
	GSIClientNBPanelData  & p=GetPanel();
	return p.GetSocketClient();

}

#endif //#ifdef STARTLE_SERVER
