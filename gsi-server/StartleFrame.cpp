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
#  pragma implementation "StartleFrame.cpp"
#  pragma interface "StartleFrame.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#error "check this: http://wiki.codeblocks.org/index.php?title=Precompiled_headers"
#  include "wx/wx.h"
#endif

#include "wx/socket.h"
#include <wx/stopwatch.h>
#include <wx/progdlg.h>
#include <wx/menu.h>
#include <wx/ffile.h>
#include <wx/TextFile.h>

//#include <wx/fileconf.h>
#include <wx/filedlg.h>
//#include <wx/choicdlg.h>


#include "include/gsi.hpp"

#ifdef STARTLE_SERVER
#include "device.hpp"
#include "node.hpp"
#include "record.hpp"
#include "server.hpp"
#include "socket.hpp"
#include "startle.hpp"
#include "STLServerCommands.hpp"
#include "startleFrame.hpp"
#include "STLServer.hpp"
#include "microcontroller.h"
#include "DlgLUT.h"
#include "DlgLoadLUT.h"

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

// --------------------------------------------------------------------------
// resources
// --------------------------------------------------------------------------

// the application icon
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "mondrian.xpm"
#endif


class LUTToneHandler
{
public:
	LUTToneHandler(StartleFrame *Stl,DlgLUT * dlg);
	~LUTToneHandler();
	void ProcessButton(wxCommandEvent& event);

};


BEGIN_EVENT_TABLE(StartleFrame, GSIFrame)

	EVT_MENU(STL_STARTLE_TEST1, StartleFrame::OnStartleTest1)
	EVT_MENU(STL_STARTLE_TEST2, StartleFrame::OnStartleTest2)
	EVT_MENU(STL_STARTLE_LUT_HELPER, StartleFrame::OnLUTHelper)
	EVT_MENU(STL_STARTLE_LUT_LOAD, StartleFrame::OnLUTLoad)


END_EVENT_TABLE()


/************************************************************************************************
    ctor
StartleFrame::StartleFrame()
*************************************************************************************************/
StartleFrame::StartleFrame()
:GSIFrame((GSIServer *) -1)       //send nz as socket server, GSIFrame will not instantiate
{
	Init();

}

/*************************************************************************************************
    dtor
StartleFrame::~StartleFrame()
*************************************************************************************************/
StartleFrame::~StartleFrame()
{
    //default GSIFrame dtor will delete the StartleServer
	delete m_buff;
}

void StartleFrame::Init()
{
	errorType rv;

	SetSockServer(DEBUG_NEW StartleServer(GSISocketAdress,StatusBar(),TextCtrl(),this));
	m_menuSTL = DEBUG_NEW wxMenu();
	wxMenuBar * mb;
	mb=GSIFrame::GetMenuBar();
	//m_menuBar->Append(m_menuHelp, _("&Help"));
	mb->Append(m_menuSTL,_("&Startle"));


	m_menuSTL->Append(STL_STARTLE_TEST1, _("Test &1"), _("Test 1"));
	m_menuSTL->Append(STL_STARTLE_TEST2, _("Test &2"), _("Test 2"));
	m_menuSTL->Append(STL_STARTLE_LUT_HELPER, _("LUT &helper"), _("LUT helper"));
	m_menuSTL->Append(STL_STARTLE_LUT_LOAD, _("Load LUT "), _("Load LUT"));

	//m_hDev=GSINotAValidDevHandle;
	m_hNode=GSINotAValidNodeHandle;
	m_buff=DEBUG_NEW wxInt16[STLMaxADCArraySize/sizeof(GSIChar)+1];


	wxTextFile Fp(STLDefaultSineLUTFname);
	bool b;
	if(Fp.Exists())
	{
		wxString s;
		s.Printf("Loading default Sine LUT: \"%s\"\n",STLDefaultSineLUTFname);
		AppendText(s);
		b=Fp.Open();
#if GSI_USE_SHARED_POINTERS
		boost::shared_ptr<StartleLUT> lut=GetAmplitudeLUT();
		rv=lut->Load(&Fp);
#else
		rv=GetAmplitudeLUT()->Load(&Fp);
#endif
		if(rv)
			AppendText("Load Sine LUT failed!\n");
		Fp.Close();	//must close the file or else it reports it as opened!
	}


	wxTextFile FpWn(STLDefaultSineLUTFname);
	if(FpWn.Exists())
	{
		wxString s;
		s.Printf("Loading default White noise LUT: \"%s\"\n",STLDefaultNoiseLUTFname);
		AppendText(s);
#if GSI_USE_SHARED_POINTERS
		//boost::shared_ptr<StartleLUT> lut=GetNoiseAmplitudeLUT();
		rv=GetNoiseAmplitudeLUT()->Load(&FpWn);
#else
		rv=GetNoiseAmplitudeLUT()->Load(&FpWn);
#endif
		if(rv)
			AppendText("Load WhiteNoise LUT failed!\n");
		FpWn.Close();	//must close the file or else it reports it as opened!
	}

}

/*!
errorType StartleFrame::SetNumADCDataPoints(wxUint16 num_data_points)
Shouldn't be set except by the socket server, or if performing an
Immediate acquisition
*/
errorType StartleFrame::SetNumADCDataPoints(wxUint16 num_data_points)
{
	m_numADCDataPoints=num_data_points;
	if(num_data_points > STLMaxADCDataPoints)
	{
		m_numADCDataPoints=STLMaxADCDataPoints;
	}

	return errNone;
}



#if 0
The startle box will not send async records so we should be able to use the default gsSerialPort
and the standard mutex controlled record read function. It will always obtain the lock.
When we Execute(), we can read the ADC data directly from the CQueue. Must make sure the Q is large enough
to hold 4k bytes??

#endif


/***********************************************************************
StartleFrame:xxx functions

Functions of the form:
StartleFrame::Xxx(STLCommandXxx & cmd)
Are used directly by the server

Functions of the form:
errorType StartleFrame::Xxx(wxSocketBase &sock,wxInt16 q_flag,wxUint16 at_tick)
Those with a wxSocketBase &sock are those that are being requested
by a GSIClient/STLClient.
This code needs to read any extra data from the client to complete the
record.
Mustn't forget to set the q_flag and at_tick values for queued functions

************************************************************************/


#if 0
/*!
errorType	StartleFrame::Execute(STLCommandExecute & cmd)
Used directly by the server
Tell startle box to execute its set of commands
If an ADC is queued, then read the returned data

Always an IMMEDIATE command
*/

errorType	StartleFrame::Execute(STLCommandExecute & cmd)
{
errorType rv=errFail;
GSIDevice &Dev= *Node()->Device();
GSIChar recd;
size_t numRead;
errorType rv1=errNone;
//GSIChar buff[STLMaxADCArraySize+1];

	rv=Dev.Write(cmd.GetGSIRecord()) ;
	if (rv)
		return rv;


	if(IsQueuedADC())
	{
		//An ADC is queued, so will service the ADC data
		//before ADC begins, turn off the QueuedADC flag in case we abort early
		SetQueuedADC(false);	//no more ADC's. Set it here in case we abort early
//record sent. The pic is waiting for us to send an ACK, and will echo
//an ACK before starting the send

	rv=Dev.Write(GSI_ACK,&recd);		//Tell pic we are ready for the data
	if(rv)
		return rv;

//pic will now send cmd.GetNumDataPoints() data points

		rv=Dev.Read((GSIChar *const)GetADCBuffer(),GetNumADCDataPoints()*sizeof(wxInt16),&numRead,STLADCImmediateTimeout);
		SetADCNumRead(numRead); //numRead =num of bytes, will convert to num wxInt16's

//finally write a GSI_ACK to let pic know all is well

		rv1=Dev.Write(GSI_ACK,&recd);
		if(rv1)
		{
			TextCtrl()->AppendText("ADC error\nStartle box failed to echo final ACK.\n");
		}
	}
	//ADCSwapByteOrder();

	SetNumADCDataPoints(0);

	if(rv)
		return rv;

	wxString s;
	wxInt16 *const buff=GetADCBuffer();
	TextCtrl()->AppendText("First 20 ADC values\n");
	for(int i=0;i<20;i+=4)
	{
		s.Printf("%i %i %i %i\n",buff[i],buff[i+1],buff[i+2],buff[i+3]);
		TextCtrl()->AppendText(s);
	}
	WriteADCDataToDisk("STLservADC.txt",GetADCNumRead(),GetADCBuffer());
	return rv1;
}


/*!
errorType	StartleFrame::Execute(wxSocketBase & sock)
A STL_COMMAND_EXECUTE has been received from the client
through the socket interface.
Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.

Always an IMMEDIATE command
*/

errorType	StartleFrame::Execute(wxSocketBase &sock)
{
errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandExecute Cmd;

	try
	{
		STLServerCommandExecute xCmd(sock,Cmd);
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nExecute failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=Execute(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
	//setup to return ADC data
		sr.ResizeDataBuff(GetADCNumRead());
		sr.SetNumInts(GetADCNumRead());
		wxInt16 *src=GetADCBuffer();
		wxInt16 *dst=sr.GetDataArray();
		for(int i=0;i<GetADCNumRead();i++)
		{
			dst[i]=src[i];
		}
		SetADCNumRead(0);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}
#endif

#if 0
/*!
errorType StartleFrame::Reset()
Used directly by the server
Instantiate a STLCommandReset and send the record to the device
reset StartleFrame variables:
m_numADCDataPoints=0;		//no data for acquisition
m_queuedADC=false;			//no queued acquisition

Always an IMMEDIATE command
*/
errorType StartleFrame::Reset(STLCommandReset & r_Cmd)
{
errorType rv;

	rv=Node()->Device()->Write(r_Cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nReset failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}

//StarleFrame state must be updated
	m_numADCDataPoints=0;		//no data for acquisition
    m_queuedADC=false;			//no queued acquisition

	return errNone;
}


/*!
errorType StartleFrame::Reset(wxSocketBase sock)
A STL_COMMAND_RESET has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.

Always an IMMEDIATE command
*/

errorType StartleFrame::Reset(wxSocketBase &sock)
{

errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Note that the RESET command has no extra data so this is superfluous
STLCommandReset Cmd;

	try
	{
		STLServerCommandReset RCmd(sock,Cmd);
	}
	catch(gsException &exc)
	{
		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=Reset(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}
#endif

/*!
errorType StartleFrame::ResetXXX()
Used directly by the server
Instantiate a STLCommandReset and send the record to the device
reset StartleFrame variables:
m_numADCDataPoints=0;		//no data for acquisition
m_queuedADC=false;			//no queued acquisition

Always an IMMEDIATE command
*/
errorType StartleFrame::ResetXXX(STLCommandResetXXX & cmd)
{
errorType rv;
	cmd.FillGSIRecord();
	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nReset failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());
		GetStatusText()->AppendText(s);
		return rv;
	}
//StarleFrame state must be updated
	m_numADCDataPoints=0;		//no data for acquisition
    m_queuedADC=false;			//no queued acquisition

	return errNone;
}

#if 0
/*!
errorType StartleFrame::SetAmplitudeSPL(STLCommandSetAmplitudeSPL &cmd)
Used directly by the server
*/
errorType StartleFrame::SetAmplitudeSPL(STLCommandSetAmplitudeSPL &cmd)
{
errorType rv;
LUT &lut=GetAmplitudeLUT();
GSIRecord &r=cmd.GetGSIRecord();

	cmd.SetOutputByte(lut.Item(cmd.GetAmplitudeSPL()));

	r.SetData(STL_REC_OFFSET_P0_LSB,cmd.GetOutputByte());

	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nSetAmplitudeSPL failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}

	return errNone;

}


/*!
errorType StartleFrame::SetAmpitudeSPL(wxSocketBase sock)
A STL_COMMAND_SET_AMPLITUDE_SPL has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.
*/

errorType StartleFrame::SetAmplitudeSPL(wxSocketBase &sock,wxInt16 q_flag,wxUint16 at_tick)
{
errorType rv;
GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandSetAmplitudeSPL Cmd(0,q_flag,at_tick); //dummy value for amplitude

	try
	{
		STLServerCommandSetAmplitudeSPL SPLCmd(sock,Cmd,GetAmplitudeLUT());
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nSetAmplitudeSPLXXX failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=SetAmplitudeSPL(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}

#endif

/*!
errorType StartleFrame::SetAmplitudeSPLXXX(STLCommandSetAmplitudeSPLXXX &cmd)
Used directly by the server
*/
errorType StartleFrame::SetAmplitudeSPLXXX(STLCommandSetAmplitudeSPLXXX &cmd)
{
errorType rv;
	cmd.FillGSIRecord();
	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nSetAmplitudeSPL failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());
		GetStatusText()->AppendText(s);
		return rv;
	}

	return rv;
}

// --- End StartleFrame::SetAmplitudeSPLXXX(

/*!
errorType StartleFrame::SetChannelXXX(STLCommandSetChannelXXX &cmd)
Used directly by the server
*/
errorType StartleFrame::SetChannelXXX(STLCommandSetChannelXXX &cmd)
{
errorType rv;
	cmd.FillGSIRecord();
	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nSetChannelXXX failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());
		GetStatusText()->AppendText(s);
		return rv;
	}

	return rv;
}

// --- End StartleFrame::SetChannelXXX


/*!
errorType StartleFrame::ExecuteXXX(STLCommandExecuteXXX &cmd)
Used directly by the server
*/
errorType StartleFrame::ExecuteXXX(STLCommandExecuteXXX &cmd)
{
errorType rv=errFail;
GSIDevice &Dev= *Node()->Device();
GSIChar recd;
size_t numRead=0;
errorType rv1=errNone;
//GSIChar buff[STLMaxADCArraySize+1];

	cmd.FillGSIRecord();
	rv=Dev.Write(cmd.GetGSIRecord()) ;
	if (rv)
		return rv;

	if(IsQueuedADC())
	{
		//An ADC is queued, so will service the ADC data
		//before ADC begins, turn off the QueuedADC flag in case we abort early
		SetQueuedADC(false);	//no more ADC's. Set it here in case we abort early
//record sent. The pic is waiting for us to send an ACK, and will echo
//an ACK before starting the send

	rv=Dev.Write(GSI_ACK,&recd);		//Tell pic we are ready for the data
	if(rv)
		return rv;

//pic will now send cmd.GetNumDataPoints() data points

		rv=Dev.Read((GSIChar *const)GetADCBuffer(),GetNumADCDataPoints()*sizeof(wxInt16),&numRead,STLADCImmediateTimeout);
		SetADCNumRead(numRead); //numRead =num of bytes, will convert to num wxInt16's

//finally write a GSI_ACK to let pic know all is well

		rv1=Dev.Write(GSI_ACK,&recd);
		if(rv1)
		{
			TextCtrl()->AppendText("ADC error\nStartle box failed to echo final ACK.\n");
		}
	}
	//ADCSwapByteOrder();

	SetNumADCDataPoints(0);

	if(rv)
		return rv;
	if(numRead)
	{
		wxString s;
		wxInt16 *const buff=GetADCBuffer();
		TextCtrl()->AppendText("First 20 ADC values\n");
		for(int i=0;i<20;i+=4)
		{
			s.Printf("%i %i %i %i\n",buff[i],buff[i+1],buff[i+2],buff[i+3]);
			TextCtrl()->AppendText(s);
			if(i > (int) numRead)	//guaranteed to be <= 65535
				break;
		}
		WriteADCDataToDisk("STLservADC.txt",GetADCNumRead(),GetADCBuffer());
	}
	return rv1;

}

// --- End StartleFrame::Execute-----------------


/*!
errorType StartleFrame::SetNoiseAmplitudeSPLXXX(STLCommandSetNoiseAmplitudeSPLXXX &cmd)
Used directly by the server
*/
errorType StartleFrame::SetNoiseAmplitudeSPLXXX(STLCommandSetNoiseAmplitudeSPLXXX &cmd)
{
errorType rv;

	cmd.FillGSIRecord();
	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nSetNoiseAmplitudeSPLXXX failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());
		GetStatusText()->AppendText(s);
		return rv;
	}

	return rv;
}

// --- End StartleFrame::SetNoiseAmplitudeSPLXXX(

#if 0

#endif

/*!
errorType StartleFrame::SetFrequencyXXX(STLCommandSetFrequency &f_Cmd)
Used directly by the server
*/
errorType StartleFrame::SetFrequencyXXX(STLCommandSetFrequencyXXX &cmd)
{
errorType rv;

	cmd.FillGSIRecord();
	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nSetFrequencyXXX failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());
		GetStatusText()->AppendText(s);

		return rv;
	}

	return errNone;
}

#if 0
/*!
errorType StartleFrame::SetFrequency(wxSocketBase sock)
A STL_COMMAND_SET_FREQUENCY has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.
*/

errorType StartleFrame::SetFrequency(wxSocketBase &sock,wxInt16 q_flag,wxUint16 at_tick)
{

errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandSetFrequency Cmd(0,q_flag,at_tick); //dummy value for frequency

	try
	{
		STLServerCommandSetFrequency FCmd(sock,Cmd); //frequency read here
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nSetFrequency failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=SetFrequency(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}



/*!
errorType	StartleFrame::Acquire(STLCommandAcquire & cmd)
Used directly by the server
If queued, return immediately after sending the command
as this will be performed in Execute()
Otherwise Acquire...
*/

errorType	StartleFrame::Acquire(STLCommandAcquire & cmd)
{
errorType rv=errFail;
GSIDevice &Dev= *Node()->Device();
GSIChar recd;
size_t numRead;
//GSIChar buff[STLMaxADCArraySize+1];
errorType rv1;


	rv=Dev.Write(cmd.GetGSIRecord()); //write the command
	if (rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(err.GetErrorCode());

	}

	if(cmd.GetQueuedFlag()==STL_QUEUED)
	{
		SetNumADCDataPoints(cmd.GetNumDataPoints());
		SetQueuedADC(true);
		return errNone;		//nothing to do, the command is queued
	}
//record sent. The pic is waiting for us to send an ACK, and will echo
//an ACK before starting the send

	rv=Dev.Write(GSI_ACK,&recd);
	if(rv)
	{
	ErrorClass err(rv);

		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(err.GetErrorCode());

	}

//pic will now send cmd.GetNumDataPoints() data points
	//virtual errorType Read(GSIChar *buff,size_t num,size_t *num_read,long timeout) const =0;
	//pic sends lowbyte:high byte so should read correctly without a byte swap
	rv=Dev.Read((GSIChar * const)GetADCBuffer(),cmd.GetNumDataPoints()*sizeof(wxInt16),&numRead,STLADCImmediateTimeout);
//finally write a GSI_ACK to let pic know all is well
	SetADCNumRead(numRead);

	rv1=Dev.Write(GSI_ACK,&recd);

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(err.GetErrorCode());
	}

	WriteADCDataToDisk("STLservADC.txt",GetADCNumRead(),GetADCBuffer());
	//swap byte order
	//ADCSwapByteOrder();

	return rv1;
}

//----------End StartleFrame::Acquire()
#endif

/*!
errorType	StartleFrame::AcquireXXX(STLCommandAcquireXXX & cmd)
Used directly by the server
If queued, return immediately after sending the command
as this will be performed in Execute()
Otherwise Acquire...
*/

errorType	StartleFrame::AcquireXXX(STLCommandAcquireXXX & cmd)
{
errorType rv=errFail;
GSIDevice &Dev= *Node()->Device();
GSIChar recd;
size_t numRead;

errorType rv1;

	cmd.FillGSIRecord();
	rv=Dev.Write(cmd.GetGSIRecord()); //write the command
	if (rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(err.GetErrorCode());

	}

	if(cmd.GetQueuedFlag()==STL_QUEUED)
	{
		SetNumADCDataPoints(cmd.GetNumDataPoints());
		SetQueuedADC(true);
		return errNone;		//nothing to do, the command is queued
	}
//record sent. The pic is waiting for us to send an ACK, and will echo
//an ACK before starting the send

	rv=Dev.Write(GSI_ACK,&recd);
	if(rv)
	{
	ErrorClass err(rv);

		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(err.GetErrorCode());

	}

//pic will now send cmd.GetNumDataPoints() data points
	//virtual errorType Read(GSIChar *buff,size_t num,size_t *num_read,long timeout) const =0;
	//pic sends lowbyte:high byte so should read correctly without a byte swap
	rv=Dev.Read((GSIChar * const)GetADCBuffer(),cmd.GetNumDataPoints()*sizeof(wxInt16),&numRead,STLADCImmediateTimeout);
//finally write a GSI_ACK to let pic know all is well
	SetADCNumRead(numRead);

	rv1=Dev.Write(GSI_ACK,&recd);

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(err.GetErrorCode());
	}

	WriteADCDataToDisk("STLservADC.txt",GetADCNumRead(),GetADCBuffer());
	//swap byte order
	//ADCSwapByteOrder();

	return rv1;
}


//-----End StartleFrame::AcquireXXX()


/*!
void StartleFrame::ADCSwapByteOrder()
*/
void StartleFrame::ADCSwapByteOrder()
{
	wxInt16 *const array=GetADCBuffer();
	wxInt16 temp;

	for(int i=0;i<GetADCNumRead();i++)
	{
		//array[i]=buff[i+i]>>8;
		//array[i]+=buff[i+i+1];
		temp=(array[i] & 0xff) << 8;		//isolate low byte and shift to upper byte
		array[i] >>= 8;
		array[i]+=temp;
	}
}

#if 0
/*!
errorType StartleFrame::Acquire(wxSocketBase sock)
A STL_COMMAND_ACQUIRE has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.
*/

errorType StartleFrame::Acquire(wxSocketBase &sock,wxInt16 q_flag,wxUint16 at_tick)
{

errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandAcquire Cmd(0,q_flag,at_tick); //dummy value for num_data_points

	try
	{
		STLServerCommandAcquire ACmd(sock,Cmd); //num_data_points read here
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nAcquire failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=Acquire(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the record and/or received ADC data from the box
		if(q_flag==STL_IMMEDIATE)
		{
		//ADC data is in buffer
		//setup to return ADC data
			sr.ResizeDataBuff(GetADCNumRead());
			sr.SetNumInts(GetADCNumRead());
			wxInt16 *src=GetADCBuffer();
			wxInt16 *dst=sr.GetDataArray();
			for(int i=0;i<GetADCNumRead();i++)
			{
				dst[i]=src[i];
			}
			SetADCNumRead(0);
		}
		sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}




/*!
errorType StartleFrame::WaitForCompletion(STLCommandWaitForCompletion &f_Cmd)

Always an IMMEDIATE command
*/
errorType StartleFrame::WaitForCompletion(STLCommandWaitForCompletion &w_Cmd)
{
errorType rv=errTimeout;
wxStopWatch sw;
wxInt16 timeout;
bool to=true;
STLCommandGetStatus &SCmd=w_Cmd.GetStatusCommand();

    timeout=STLADCImmediateTimeout;
    do
    {
        if(GetStatus(SCmd))
        {
            ErrorClass Err(rv);
            wxString s;
            s.Printf("WaitForCompletion: failed. Error code=%i (%s)\n",Err.GetErrorCode().c_str(),Err.GetString().c_str());
            TextCtrl()->AppendText(s);
            return(rv);
        }
		if(SCmd.GetStatus() || SCmd.GetTMR0On() || SCmd.GetExecuteCounter() ==0)
		{
			to=false;
			rv=errNone;
			break;
		}

    }while (sw.Time() < timeout);


    if(to)
    {
        TextCtrl()->AppendText(_("WaitForCompletion: failed (timeout error)\n"));
    }
    else
    {
        TextCtrl()->AppendText(_("WaitForCompletion: success\n"));
    }

    return(rv);
}




/*!
errorType StartleFrame::WaitForCompletion(wxSocketBase sock)
A STL_COMMAND_WAIT_FOR_COMPLETION has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.

Always an IMMEDIATE command
*/

errorType StartleFrame::WaitForCompletion(wxSocketBase &sock)
{

errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandWaitForCompletion Cmd;

	try
	{
		STLServerCommandWaitForCompletion FCmd(sock,Cmd);
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nWaitForCompletion failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=WaitForCompletion(Cmd);			//send command (GetStatus) to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}

#endif

/*!
errorType StartleFrame::WaitForCompletionXXX(STLCommandSetFrequency &f_Cmd)
Used directly by the server

Always an IMMEDIATE command
*/
errorType StartleFrame::WaitForCompletionXXX(STLCommandWaitForCompletionXXX &cmd)
{
errorType rv=errTimeout;
wxStopWatch sw;
wxInt16 timeout;
bool to=true;
STLCommandGetStatusXXX SCmd;

	cmd.FillGSIRecord();
    timeout=STLADCImmediateTimeout;
    do
    {
        if(GetStatusXXX(SCmd))
        {
            ErrorClass Err(rv);
            wxString s;
            s.Printf("WaitForCompletion: failed. Error code=%i (%s)\n",Err.GetErrorCode(),Err.GetString().c_str());
            TextCtrl()->AppendText(s);
            return(rv);
        }
		if(SCmd.GetStatus() || SCmd.GetTMR0On() || SCmd.GetExecuteCounter() ==0)
		{
			to=false;
			rv=errNone;
			break;
		}

    }while (sw.Time() < timeout);


    if(to)
    {
        TextCtrl()->AppendText(_("WaitForCompletion: failed (timeout error)\n"));
    }
    else
    {
        TextCtrl()->AppendText(_("WaitForCompletion: success\n"));
    }

    return(rv);
}





//----End StartleFrame::WaitForCompletionXXX

#if 0
/*!
errorType StartleFrame::GetStatus(STLCommandGeStatus &s_Cmd)
Used directly by the server

;		data[0]=STL_COMMAND_GET_STATUS
;Return current state of the pic
bit positions for GlbStatus, returned in rs232Record data[1]for GetStatus
also returns data[2]:0 bit, set when performing a timed tone
returns:
data[0]
const   wxUint16 STLStatusADCDataReady	=0x1;       //bit 0
const   wxUint16 STLStatusADCAcquiring	=0x2;
const   wxUint16 STLStatusADCEnd         =0x4;
const   wxUint16 STLStatusADCQueued      =0x8;
const   wxUint16 STLStatusExecuteAt      =0x10;
const   wxUint16 STLStatusToneStart      =0x20;       //a tone is on
const   wxUint16 STLStatusBit6           =0x40;
const   wxUint16 STLStatusErrorR2Overflow=0x80;
//msb

data[1] bit shifted to bit 0 position
const   wxUint16 STLtatusTimedTone       =0x100;   //a timed tone is on

data[2]
ExecuteCounter  Number of startle commands still to execute

Always an IMMEDIATE command
*/
errorType StartleFrame::GetStatus(STLCommandGetStatus &Cmd)
{
errorType rv;
	//send command to box
	rv=Node()->Device()->Write(Cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nGetStatus failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		 return rv;
	}
	if(Cmd.GetGSIRecord().IsReturnedRecord())
	{
		GSIRecord * rr=Cmd.GetGSIRecord().GetReturnedRecord();
		Cmd.SetStatus(rr->GetData(1));		//glbGtStatus from startle box
		Cmd.SetTMR0On(rr->GetData(2));		//bit 0=TMR on (set=on)
		Cmd.SetExecuteCounter(rr->GetData(3));//num commands still to execute
	}

	if(Cmd.GetStatus() &0x80)
	{
		wxMessageBox(_("RS232 buffer overflowed in startle box"),
			_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return errADCRead;
	}

	return errNone;
}



/*!
errorType StartleFrame::GetStatus(wxSocketBase sock)
A STL_COMMAND_GET_STATUS has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.

Always an IMMEDIATE command
*/

errorType StartleFrame::GetStatus(wxSocketBase &sock)
{

errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandGetStatus Cmd;

	try
	{
		STLServerCommandGetStatus SCmd(sock,Cmd);
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nGeStatus failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=GetStatus(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}
#endif


/*!
	errorType StartleFrame::GetStatusXXX(STLCommandGetStatusXXX &Cmd)

Used directly by the server

;		data[0]=STL_COMMAND_GET_STATUS
;Return current state of the pic
bit positions for GlbStatus, returned in rs232Record data[1]for GetStatus
also returns data[2]:0 bit, set when performing a timed tone
returns:
data[0]
const   wxUint16 STLStatusADCDataReady	=0x1;       //bit 0
const   wxUint16 STLStatusADCAcquiring	=0x2;
const   wxUint16 STLStatusADCEnd         =0x4;
const   wxUint16 STLStatusADCQueued      =0x8;
const   wxUint16 STLStatusExecuteAt      =0x10;
const   wxUint16 STLStatusToneStart      =0x20;       //a tone is on
const   wxUint16 STLStatusBit6           =0x40;
const   wxUint16 STLStatusErrorR2Overflow=0x80;
//msb

data[1] bit shifted to bit 0 position
const   wxUint16 STLtatusTimedTone       =0x100;   //a timed tone is on

data[2]
ExecuteCounter  Number of startle commands still to execute

Always an IMMEDIATE command
*/

errorType StartleFrame::GetStatusXXX(STLCommandGetStatusXXX &Cmd)
{
errorType rv;
	//send command to box

	Cmd.FillGSIRecord();
	rv=Node()->Device()->Write(Cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nGetStatus failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());
		GetStatusText()->AppendText(s);
		 return rv;
	}
	if(Cmd.GetGSIRecord().IsReturnedRecord())
	{
		GSIRecord * rr=Cmd.GetGSIRecord().GetReturnedRecord();
		Cmd.SetStatus(rr->GetData(1));		//glbGtStatus from startle box
		Cmd.SetTMR0On(rr->GetData(2));		//bit 0=TMR on (set=on)
		Cmd.SetExecuteCounter(rr->GetData(3));//num commands still to execute
	}

	if(Cmd.GetStatus() &0x80)
	{
		wxMessageBox(_("RS232 buffer overflowed in startle box"),
			_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return errADCRead;
	}

	return errNone;
}

//------End StartleFrame::GetStatusXXX--------------


#if 0
/*!
errorType StartleFrame::SetChannel(STLCommandSetChannel &cmd)
Used directly by the server
*/
errorType StartleFrame::SetChannel(STLCommandSetChannel &cmd)
{
errorType rv;

	rv=Node()->Device()->Write(cmd.GetGSIRecord());
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nSetChan failed on Device:Node:%i Device:%s",err.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return rv;
	}

	return errNone;
}


/*!
errorType StartleFrame::SetChan(wxSocketBase sock)
A STL_COMMAND_SET_CHAN has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.
*/

errorType StartleFrame::SetChannel(wxSocketBase &sock,wxInt16 q_flag,wxUint16 at_tick)
{

errorType rv;

GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandSetChannel Cmd(0,q_flag,at_tick); //dummy value for channel

	try
	{
		STLServerCommandSetChannel CCmd(sock,Cmd,GetStatusText());	//channel read here
	}
	catch(gsException &exc)
	{
		wxString s;
		s.Printf("%s\nSetChannel failed on Device:Node:%i Device:%s",exc.GetString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(exc.GetErrorCode());
	}

//ok rest of command is in record
	rv=SetChannel(Cmd);			//send command to startle box
	if(rv==errNone)
    {
    //we have succesfully sent the data
        sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
        sr.Write();
    }
    else
    {
        sr.Write();		//will write failure
    }

	return rv;
}


/*!
errorType StartleFrame::SetAmplitudeLUTValue(STLCommandAmplitudeLUTValue &cmd)
Used directly by the server
Always an IMMEDIATE command
*/
errorType StartleFrame::SetAmplitudeLUTValue(STLCommandSetAmplitudeLUTValueXXX &cmd)
{

	GetAmplitudeLUT().Replace(cmd.GetIndex(),cmd.GetValue());
	return errNone;
}


/*!
errorType StartleFrame::SetAmplitudeLUTValue(wxSocketBase sock)
A STL_COMMAND_SET_AMPLITUDE_LUT has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.
*/

errorType StartleFrame::SetAmplitudeLUTValue(wxSocketBase &sock)
{
GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandSetAmplitudeLUTValue Cmd(0,0);	//dummy values to keep ctor happy

//This ctor will not throw
	STLServerCommandSetAmplitudeLUTValue LCmd(sock,Cmd); //index,value read here

    //we have succesfully sent the data
    sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
    sr.Write();

	return errNone;
}



/*!
errorType StartleFrame::SetNoiseLUTValue(STLCommandSetNoiseLUTValue &cmd)
Used directly by the server
*/
errorType StartleFrame::SetNoiseAmplitudeLUTValue(STLCommandSetNoiseAmplitudeLUTValue &cmd)
{

	GetNoiseAmplitudeLUT().Replace(cmd.GetIndex(),cmd.GetValue());
	return errNone;
}


/*!
errorType StartleFrame::SetNoiseLUTValue(wxSocketBase sock)
A STL_COMMAND_SET_NOISE_LUT has been received from the client
through the socket interface.

Called from STLServer, need to read the rest of the data
from the socket (if any), execute the command by writing a
GSIRecord to the startle box, and return an error code
to the client through the socket.
*/

errorType StartleFrame::SetNoiseAmplitudeLUTValue(wxSocketBase &sock)
{
GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL);       //tell client we have success/failure

//first read the rest of the command if any
//Instantiate the command, the ctor will read from the socket and
//throw exception if problem
STLCommandSetNoiseAmplitudeLUTValue Cmd(0,0);	//dummy values to keep ctor happy

//This ctor will not throw
	STLServerCommandSetNoiseAmplitudeLUTValue LCmd(sock,Cmd); //index,value read here

    //we have succesfully sent the data
    sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
    sr.Write();

	return errNone;
}

#endif

/*!
errorType StartleFrame::SetNoiseAmplitudeSPLXXX(STLCommandSetNoiseAmplitudeSPLXXX &cmd)
Used directly by the server
*/
errorType StartleFrame::SetNoiseAmplitudeLUTValueXXX(STLCommandSetNoiseAmplitudeLUTValueXXX &cmd)
{
errorType rv=errFail;
#if GSI_USE_SHARED_POINTERS
boost::shared_ptr<StartleLUT>lut=GetNoiseAmplitudeLUT();
#else
StartleLUT * const lut=GetNoiseAmplitudeLUT();
#endif
	lut;
	cmd;
	return rv;
}


/*!
errorType StartleFrame::SetAmplitudeSPLXXX(STLCommandSetAmplitudeSPLXXX &cmd)
Used directly by the server
*/
errorType StartleFrame::SetAmplitudeLUTValueXXX(STLCommandSetAmplitudeLUTValueXXX &cmd)
{
errorType rv=errFail;


#if GSI_USE_SHARED_POINTERS
	boost::shared_ptr<StartleLUT> lut=GetAmplitudeLUT();
#else
	LUT *const lut=GetAmplitudeLUT();
#endif
	rv=lut->Replace(cmd.GetIndex(),cmd.GetValue());
	return rv;
}

/* END StartleFrame*/


/**************************************************
	StartleFrame::OnStartleTest1(wxCommandEvent& event)

***************************************************/
void StartleFrame::OnStartleTest1(wxCommandEvent& event)
{
	wxFAIL_MSG("Not implemented");
#if 0
errorType rv;
GSIDevHandle hdev;
wxString devStr("Com6");

	rv=OpenDevice(devStr, &hdev);
	if(rv)
	{
		 wxMessageBox(_("Couldn't open device"),
				_("Error"),
				wxOK | wxICON_EXCLAMATION, this);
		return;
	}
	rv=CreateNode(hdev);
	if(rv)
	{
		ErrorClass err(rv);
		wxString s=err.GetString();

		 wxMessageBox(_("Couldn't open Node on device"),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		CloseDevice(hdev);
		return;
	}

	TextCtrl()->AppendText("Sending RESET to startle box\n");
	STLCommandReset rCmd;
	GSIRecord &r=rCmd.GetGSIRecord();
	rv=Node()->Device()->Write(r);
	if(rv)
	{
		ErrorClass err(rv);
		wxString s=err.GetString();

		 wxMessageBox(s,
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		CloseDevice(hdev);
		return;
	}

	TextCtrl()->AppendText("Performing Frequency scan 1kHz to 20kHz 200Hz intervals\n");
	for(int j=1000; j<= 20000;j+=200)
	{
		{
			STLCommandSetFrequency sfCmd(j);

			GSIRecord & r=sfCmd.GetGSIRecord();

			rv=Node()->Device()->Write(r);
		}
	}
	TextCtrl()->AppendText("Done\nSetting frequency to 5kHz\n");

	STLCommandSetFrequency sfCmd(5000);
	rv=Node()->Device()->Write(sfCmd.GetGSIRecord());

	rv=Node()->Device()->Write(sfCmd.GetGSIRecord());
	TextCtrl()->AppendText("Done\n");

	TextCtrl()->AppendText("Ramping sine amplitude from 0dB to 120dB  (not working properly yet)\n");
	for(int i=0;i<120;i++)
	{
		{
			STLCommandSetAmplitudeSPL saCmd(i);
			rv=Node()->Device()->Write(saCmd.GetGSIRecord());
		}
	}
	TextCtrl()->AppendText("Leaving at 120dB\n");

	STLCommandSetAmplitudeSPL saCmd(120);
	rv=Node()->Device()->Write(saCmd.GetGSIRecord());
	TextCtrl()->AppendText("Done\n");


	TextCtrl()->AppendText("Selecting channels 1-8\n");

	for(i=0;i<STLMaxChan;i++)
	{
		{
			STLCommandSetChannel scCmd(i);
			rv=Node()->Device()->Write(scCmd.GetGSIRecord());
		}
	}

	CloseDevice(hdev);
#endif
	event;
}


/**************************************************
	StartleFrame::OnStartleTest1(wxCommandEvent& event)

***************************************************/
void StartleFrame::OnStartleTest2(wxCommandEvent& event)
{
event;

errorType rv;
GSIDevHandle hDev;
GSINodeHandle hNode=GSI_NODE_THIS;
wxString devStr("Com6");
wxString str;
GSIDevice * gsiDevice;
STLQueuedFlag qFlag=STL_IMMEDIATE;
wxUint16 tick=0;
int i=0;

	i;
	TextCtrl()->AppendText("Testing Startle commands\n");

//------------Open Device--------------------
	str.Printf("Opening %s\n",devStr.c_str());
	TextCtrl()->AppendText(str);

	rv=OpenDevice(devStr, &hDev);
	if(rv && rv != errAlreadyOpen)
	{
		 wxMessageBox(_("Couldn't open device"),
				_("Error"),
				wxOK | wxICON_EXCLAMATION, this);
		return;
	}

//---------Open Node-----------------------------------
	str.Printf("Opening node %i\n",hNode);
	TextCtrl()->AppendText(str);

	rv=CreateNode(hDev,hNode);
	if(rv && rv != errAlreadyOpen)
	{
		ErrorClass err(rv);
		wxString s=err.GetString();

		 wxMessageBox(_("Couldn't open Node on device"),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		CloseDevice(hDev);
		return;
	}

	gsiDevice=Node()->Device();
//-----------STARTLE TEST---------------------------------

	//STL_COMMAND_RESET
	STLCommandResetXXX rCmd;
	rCmd.SetStatusText(TextCtrl());
	rv=ResetXXX(rCmd);
	if(rv)
	{
		CloseDevice(hDev);
		return;
	}

//----------STL_COMMAND_SET_AMPLITUDE_SPL----------
	{
		STLCommandSetAmplitudeSPLXXX Cmd(70,GetAmplitudeLUT(),qFlag,tick);
		rCmd.SetStatusText(TextCtrl());
		rv=SetAmplitudeSPLXXX(Cmd);		//will write to startle box
		if(rv)
		{
			CloseDevice(hDev);
			return;
		}
	}

//----------STL_COMMAND_SET_FREQUENCY----------
	{
		STLCommandSetFrequencyXXX Cmd(5500,qFlag,tick+=10);
		rCmd.SetStatusText(TextCtrl());
		rv=SetFrequencyXXX(Cmd);
		if(rv)
		{
			CloseDevice(hDev);
			return;

		}
	}

//----------STL_COMMAND_SET_CHANNEL----------
	{
		STLCommandSetChannelXXX Cmd(0,qFlag,tick+=10);
		rCmd.SetStatusText(TextCtrl());
		rv=SetChannelXXX(Cmd);
	}
	if(rv)
	{
		CloseDevice(hDev);
		return;

	}


//----------STL_COMMAND_ACQUIRE----------
//Have to use a STLCommandAcquire of function scope as we need to send it
//as a parameter to Execute(), so mustn't destroy it in a code block
	{
		STLCommandAcquireXXX Cmd(1000,qFlag,tick+=100);
		rCmd.SetStatusText(GetStatusText());
		rv=AcquireXXX(Cmd);
		if(rv)
		{
			CloseDevice(hDev);
			return;
		}
	}
			//GSIRecord & r=Cmd.GetGSIRecord();
//Once we write the Acquire command, the pic will wait for us to send an ACK
//before starting acquisition and sending us data.
//Thus, we must write a function to handle the Acq
			//rv=Node()->Device()->Write(r);



//----------STL_SET_AMPLITUDE_SPL----------
	STLCommandSetAmplitudeSPLXXX Cmd(0,GetAmplitudeLUT(),qFlag,tick+=10);
	rCmd.SetStatusText(GetStatusText());
	rv=SetAmplitudeSPLXXX(Cmd);
	if(rv)
	{
		CloseDevice(hDev);
		return;
	}

//----------STL_COMMAND_GET_STATUS----------
	STLCommandGetStatusXXX SCmd(0,qFlag);
	rCmd.SetStatusText(GetStatusText());
	rv=GetStatusXXX(SCmd);
	if(rv)
	{
		GSIRecord * rr=SCmd.GetGSIRecord().GetReturnedRecord();
		SCmd.SetStatus(rr->GetData(1));
		SCmd.SetTMR0On(rr->GetData(2));
		SCmd.SetExecuteCounter(rr->GetData(3));
	}
	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\nGetStatus failed on Device:Node:%i Device:%s",err.GetString().c_str(),hNode,devStr.c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		CloseDevice(hDev);
		return;

	}


//----------STL_SET_NOISE_AMPLITUDE_SPL----------
	{
			STLCommandSetNoiseAmplitudeSPLXXX Cmd(80,GetNoiseAmplitudeLUT(),qFlag,tick+=10);
			rCmd.SetStatusText(GetStatusText());
			rv=SetNoiseAmplitudeSPLXXX(Cmd);
	}
	if(rv)
	{
		CloseDevice(hDev);
		return;
	}


//----------STL_SET_NOISE_AMPLITUDE_SPL----------
	{
	STLCommandSetNoiseAmplitudeSPLXXX Cmd(0,GetNoiseAmplitudeLUT(),qFlag,tick+=10);
		rCmd.SetStatusText(GetStatusText());
		rv=SetNoiseAmplitudeSPLXXX(Cmd);
	}
	if(rv)
	{
		CloseDevice(hDev);
		return;
	}


//----------STL_SET_AMPLITUDE_LUT_VALUE----------

	for(i=0;i<STLMaxLUTIndex;i++)
	{
		STLCommandSetAmplitudeLUTValueXXX Cmd(i,i,qFlag); //index,value
		rv=SetAmplitudeLUTValueXXX(Cmd);
		if(rv)
		{
			CloseDevice(hDev);
			return;
		}
	}
#if 0
//----------STL_SET_NOISE_LUT----------
	for(i=0;i<STLMaxLUTIndex;i++)
	{
		STLCommandSetNoiseAmplitudeLUTValueXXX Cmd(i,i,qFlag);
		rv=SetNoiseAmplitudeLUTValueXXX(Cmd);

		if(rv)
		{
			CloseDevice(hDev);
			return;
		}
	}
#endif
//----------STL_COMMAND_EXECUTE----------
	{
		STLCommandExecuteXXX Cmd(0,qFlag);
		rCmd.SetStatusText(GetStatusText());
		ExecuteXXX(Cmd);
	}
	if(rv)
	{
		CloseDevice(hDev);
		return;

	}
//----------STL_COMMAND_WAIT_FOR_COMPLETION----------
	{
		STLCommandWaitForCompletionXXX Cmd(0,qFlag);
		rCmd.SetStatusText(GetStatusText());
		rv=WaitForCompletionXXX(Cmd);
	}
	if(rv)
	{
		CloseDevice(hDev);
		return;
	}


#if 0
STL_COMMAND_RESET=0,                      //Use WaitForCompletion if startle box is running, this will stop all running tasks in box
STL_COMMAND_SET_AMPLITUDE=1,               //Set the Amplitude DAC, p0=0-STLMaxAmplitudeDAC
STL_COMMAND_SET_FREQUENCY=2,				//used in conjuction with STL_COMMAND_SET_FREQUENCY_UPPER for DDS version
STL_COMMAND_SET_DELAY=3,
STL_COMMAND_ACQUIRE=4,
STL_COMMAND_SET_CHANNEL=5,
STL_COMMAND_EXECUTE=6,
STL_COMMAND_TONE=7,                       //p0=
STL_COMMAND_TIMED_TONE_ON=8,                //p0=period,p1=prescaler,p2=at_tick
STL_COMMAND_SET_NOISE_AMPLITUDE=9,          //Set the Noise DAC, p0=0-STLMaxNoiseAmplitudeDAC
STL_COMMAND_GET_STATUS=10,                 //Used internally. See StatusBits if you really want to use it
STL_COMMAND_SET_FREQUENCY_UPPER=11,			//V2 specific
//reserve 10 commands for more external (outbound to controller) commands


//Start of internal commands. Commands the server will handle or fiddle with
STL_COMMAND_WAIT_FOR_COMPLETION=21,          //po=timeout (max 32767) in msec. Waits till the startle box has finished all its tasks. Uses the outbound GetStatus()
STL_COMMAND_OPEN_DEVICE=22,                  //open the comport, p0=comport number (1-6)

STL_COMMAND_SET_AMPLITUDE_SPL=23,           //p0=dB 1-126, 126 is max output from amp
STL_COMMAND_SET_NOISE_AMPLITUDE_SPL=24,      //p0=dB 1-126, 126 is max output from amp
STL_COMMAND_SET_FREQUENCY_HZ=25,            //p0=f (STLMinFrequency-STLMaxFrequency)

STL_COMMAND_SET_AMPLITUDE_LUT=26,           //set an element of the LUT p0=index, p1=value
STL_COMMAND_SET_NOISE_LUT=27,                //set an element of the LUT p0=index, p1=value
STL_COMMAND_CLOSE_DEVICE=28

#endif



//------------close node---------------------
	str.Printf("Closing node %i\n",hNode);
	TextCtrl()->AppendText(str);

	rv=RemoveNode(hNode);

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s.\n Couldn't close Node:%i on Device:%s",err.GetString().c_str(),hNode,devStr.c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		CloseDevice(hDev);
		return;
	}

//------------close device---------------------
	str.Printf("Closing device %s\n",devStr.c_str());
	TextCtrl()->AppendText(str);

	rv=CloseDevice(hDev);

	if(rv)
	{
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s",err.GetString().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		return;
	}
}


/*!
errorType	StartleFrame::WriteADCDataToDisk(const wxString & fname,wxInt16 num_ints,wxInt16 *const buffer)

NOTE: Notepad on winxp doesn't seem to like the text file generated by this
routine. Notepad++ doesn't have any problems.
*/
errorType	StartleFrame::WriteADCDataToDisk(const wxString & fname,wxInt16 num_ints,wxInt16 *const buffer)
{
wxString s;
wxString s1="";
int i;

	wxFFile ofile(fname,"wt");
	if(ofile.IsOpened())
	{
		for(i=0;i<num_ints;i++)
		{
			s.Printf("%i\n",buffer[i]);
			s1+=s;
		}
		ofile.Write(s1);
		ofile.Close();
	}
	else
		return errFail;


	return errNone;
}



/*!
	errorType GSIFrame::ProcessSTLCommand(GSIServerCommandXXX &Cmd)

Receive polymorphic class and process it.
A server class means that there is still class specific data to be
read from the socket.
The polymorphic Read() function knows how to handle it.

Note that the Execute() command will be called with a StartleFrame &
And can therefore customise the StartleFrame
*/

errorType StartleFrame::ProcessServerCommand(STLCommandXXX &Cmd,wxSocketBase &sock)
{
errorType rv;
GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_FAIL,false);       //tell client we have success/failure
wxString s;

	s.Printf("Reading %s record from socket\n",Cmd.GetCommandString().c_str() );
	Cmd.AppendText(s);
	rv=Cmd.Read(sock);	//though this can call a polymorphic function that has access to the sock, send it anyway so that we can use it in polymorphic classes that don't
	if(rv)
	{
		sr.SetReturnCode(GSISocketReturnData::GSI_FAIL);        //XXXGS changed to GSISocketReturnData 16/6/08
		return errFail;
	}
	switch(Cmd.GetStatusReportLevel())
	{
	case GSIStatusReportNone:
	default:
		break;
	case GSIStatusReportLow:
		break;
	case GSIStatusReportMedium:
		break;
	case GSIStatusReportHigh:
		{
			s.Printf("Read this set of data from the socket client:\n %i %s\n",GSI_SOCKET_USER1,Cmd.GetReadFromSocketString());
			Cmd.AppendText(s);
		}
		break;
	}
	rv=Cmd.Execute(*this,sr);	//execute the class specific command

	if (rv)
	{
		sr.SetReturnCode(rv); //automatically converts to GSISocketReturnValue
		ErrorClass err(rv);
		wxString s;
		s.Printf("%s\n%s on Device:Node:%i Device:%s",err.GetString().c_str(),Cmd.GetErrorString().c_str(),Node()->GetNodeHandle(),Node()->Device()->GetDeviceName().c_str());

		 wxMessageBox(_(s),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);

		return(rv);
	}
	if(sr.GetNumInts()>0)
	{
		wxString s;
		s.Printf("Returning %i 16 bit value(s) to server\n",sr.GetNumInts());
		Cmd.AppendText(s);
	}
	sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);
	return errNone;

}



void StartleFrame::OnLUTHelper(wxCommandEvent& event)
{

	event;


	DlgLUT * Dlg=DEBUG_NEW DlgLUT(this);


	int rc=Dlg->ShowModal();
	rc;
	Dlg->Destroy();
}


/*!
void StartleFrame::OnDlgLUTToneButton(wxCommandEvent& event)
User wants to test a tone in order to make a LUT entry
Need to read parameters from the dlg using text strings, convert
to int, send to node (if open). Open it if it isn't already open
*/
void StartleFrame::OnDlgLUTToneButton(wxCommandEvent& event, DlgLUT *Dlg)
{
wxComboBox * cb;
wxString s;
long data;
errorType rv;
int amp;
int time;
wxString TextStr;


	if (IsNodeOpen())
	{
		STLCommandResetXXX rCmd;
		rv=ResetXXX(rCmd);			//send reset to startle box
		if(rv)
			return;

		//get the parameters from the CB controls and send them to the startle box
		//don't forget to use text string and convert. This is the reliable method

		cb=Dlg->GetTimeCB();
		s=cb->GetValue();
		s.ToLong(&data,10);
		time=(int) data;
		if(time <= 0 || time >10000)
			return;

	//Binary value to send as volume
		cb=Dlg->GetValueCB();
		s=cb->GetValue();
		s.ToLong(&data,10);
		amp=(int) data;
		if(amp <0 || amp >STLMaxLM1972Value)
			return;

		cb=Dlg->GetDBCB();
		TextStr=cb->GetValue();

		switch(event.GetId())
		{
		case ID_BN_TONE:
			{
				//always send 5kHz
				STLCommandSetFrequencyXXX FCmd(5000,STL_IMMEDIATE,0);
				FCmd.SetStatusText(TextCtrl());
				rv=SetFrequencyXXX(FCmd);
				if(rv)
					return;

				STLCommandSetAmplitudeSPLXXX ACmd(0,STL_IMMEDIATE,0);
				ACmd.SetStatusText(TextCtrl());
				rv=SetAmplitudeSPLXXX(ACmd);
				if(rv)
					return;
				STLCommandSetAmplitudeSPLXXX AmpCmd(amp,STL_QUEUED,0);
				AmpCmd.SetStatusText(TextCtrl());
				rv=SetAmplitudeSPLXXX(AmpCmd);
				if(rv)
					return;

				AmpCmd.SetAmplitude(0);		//time shoud be @ mSec
				AmpCmd.SetAtTick(time);
				rv=SetAmplitudeSPLXXX(AmpCmd);
				if(rv)
					return;

			}
			break;
		case ID_BN_LOAD_LUT:
			Dlg;
			break;
		case ID_BN_WN:
			{
				STLCommandSetNoiseAmplitudeSPLXXX ACmd(0,STL_IMMEDIATE,0);
				ACmd.SetStatusText(TextCtrl());
				rv=SetNoiseAmplitudeSPLXXX(ACmd);
				if(rv)
					return;
				STLCommandSetNoiseAmplitudeSPLXXX AmpCmd(amp,STL_QUEUED,0);
				AmpCmd.SetStatusText(TextCtrl());
				rv=SetNoiseAmplitudeSPLXXX(AmpCmd);
				if(rv)
					return;

				AmpCmd.SetAmplitude(0);		//time shoud be @ mSec
				AmpCmd.SetAtTick(time);
				rv=SetNoiseAmplitudeSPLXXX(AmpCmd);
				if(rv)
					return;

			}
			break;

		}


		STLCommandExecuteXXX eCmd;
		rv=ExecuteXXX(eCmd);
		if(rv)
			return;
	}
	else
	{
		 wxMessageBox(_("No device open!"),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		 return;
	}
}

void StartleFrame::OnDlgLUTButton(wxCommandEvent& event, DlgLUT *Dlg)
{
wxString s;
static StartleLUT SineLUT;
static StartleLUT NoiseLUT;
int id;		//will return 0, ID_RB_SINE, ID_RB_NOISE
	Dlg;

	switch(event.GetId())
	{
	case ID_BN_TONE:
	case ID_BN_WN:
		wxFAIL;
		break;
	case ID_BN_LOAD_LUT:
		{
			wxTextFile f;
			ReadLUT(&f,&id);
		}
		break;

	case ID_BN_SAVE_SINE:
	case ID_BN_SAVE_NOISE:
		{
			wxString fname;
			wxString cwd;
			wxString path;
			cwd=::wxGetCwd();
			if(event.GetId()==ID_BN_SAVE_SINE)
				fname="Sine.lut";
			else
				fname="WhiteNoise.lut";

			wxFileDialog *FDlg=DEBUG_NEW wxFileDialog(this,"Save LUT to file",cwd,fname);
			int rc=FDlg->ShowModal();
			//probably need to write the LUT then prompt user for 2nd LUT
			if (rc==wxID_CANCEL)
			{
				FDlg->Destroy();
				return;
			}
			wxTextFile *OFile=DEBUG_NEW wxTextFile(FDlg->GetPath());
			bool b;
			if(OFile->Exists())
				b=OFile->Open();
			else
				b=OFile->Create();
			if(b==false)
			{
				wxMessageBox(_("Couldn't open file!"),
				_("Error"),
				wxOK | wxICON_EXCLAMATION, this);

				return;
			}
			if(event.GetId() == ID_BN_SAVE_SINE)
				SineLUT.Write(OFile);
			else
				NoiseLUT.Write(OFile);
			delete OFile;
		}

		break;
	}
}

void StartleFrame::OnLUTLoad(wxCommandEvent& event)
{
	wxString cwd;
	wxString path;
	cwd=::wxGetCwd();
	wxString fname;
	errorType rv;
	event;

//Can't use ReadLUT as we need result of the dialog
	wxTextFile f;
	int id;
	ReadLUT(&f,&id);

#if 0
	DlgLoadLUT * Dlg= DEBUG_NEW DlgLoadLUT(this);
	if(Dlg->ShowModal() ==wxID_OK)
	{
		switch(Dlg->GetSlctdRadioButton())
		{
		case ID_RB_SINE:
			fname="Sine.lut";
			break;
		case ID_RB_NOISE:
			fname="WhiteNoise.lut";
			break;
		}
	}
	wxFileDialog *FDlg=DEBUG_NEW wxFileDialog(this,"Load LUT",cwd,fname);
	int rc=FDlg->ShowModal();
	//probably need to write the LUT then prompt user for 2nd LUT
	FDlg->Destroy(); //destroy it straight away so no leak
	if (rc==wxID_CANCEL)
	{
		Dlg->Destroy();
		return;
	}
	wxTextFile Fp(fname);
	bool b;
	if(Fp.Exists())
		b=Fp.Open();
	else
		b=Fp.Create();
	if(b==false)
	{
		wxMessageBox(_("Couldn't open file!"),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		Dlg->Destroy();
		return;
	}
#endif
	if(id == ID_RB_SINE)
		rv=GetAmplitudeLUT()->Load(&f);
	else
		rv=GetNoiseAmplitudeLUT()->Load(&f);

	if(rv)
	{
		wxMessageBox(_("Error reading LUT"),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
	}


	//Dlg->Destroy();
}

/*!
errorType StartleFrame::ReadLUT(wxTextFile * const f)
*/
errorType StartleFrame::ReadLUT(wxTextFile * const f, int *id)
{
	wxString cwd;
	wxString path;
	cwd=::wxGetCwd();
	wxString fname;

	*id=0;

	DlgLoadLUT * Dlg= DEBUG_NEW DlgLoadLUT(this);
	if(Dlg->ShowModal() ==wxID_OK)
	{
		switch(Dlg->GetSlctdRadioButton())
		{
		case ID_RB_SINE:
			fname="Sine.lut";
			*id=ID_RB_SINE;
			break;
		case ID_RB_NOISE:
			fname="WhiteNoise.lut";
			*id=ID_RB_NOISE;
			break;
		}
	}
	wxFileDialog *FDlg=DEBUG_NEW wxFileDialog(this,"Load LUT",cwd,fname);
	int rc=FDlg->ShowModal();
	//probably need to write the LUT then prompt user for 2nd LUT
	FDlg->Destroy(); //destroy it straight away so no leak
	if (rc==wxID_CANCEL)
	{
		Dlg->Destroy();
		return errUserCancel;
	}

	if (f->Open(fname) ==false)
	{
		wxMessageBox(_("Couldn't open file!"),
		_("Error"),
		wxOK | wxICON_EXCLAMATION, this);
		Dlg->Destroy();
		return errFileOpen;
	}
	return errNone;
}

/*!
errorType StartleFrame::LoadTextFile(const wxString &fname)
Open a text file, load into memory
*/
errorType StartleFrame::LoadTextFile(const wxString &fname,const wxTextFile &fp)
{
	fp;fname;
#if 0
	wxString cwd;
	cwd=::wxGetCwd();


	if(fp.Exists())
		b=fp.Open();
	else
		b=fp.Create();

	if(b==false)
		return errFileOpen;
#endif
	return errNone;

}

#if GSI_USE_SHARED_POINTERS
boost::shared_ptr<StartleLUT> StartleFrame::GetAmplitudeLUT()
{
boost::shared_ptr<StartleLUT> lut;
	lut=GetStartleServer().GetAmplitudeLUT();
	return lut;
}
#endif

#endif //#ifdef STARTLE_SERVER


