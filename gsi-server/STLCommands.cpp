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
#  pragma implementation "STLCommands.cpp"
#  pragma interface "STLCommands.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#include "include/gsi.hpp"

#ifdef STARTLE_SERVER

#include "startle.hpp"
#include "STLCommands.hpp"
#include "microcontroller.h"
#include "startleFrame.hpp"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif

//forward reference


/*!
STLCommand::STLCommand(const wxString & Cmd_str,wxInt16 queued_flag, wxUint16 at_tick, wxInt16 node)
*/
STLCommandXXX::STLCommandXXX()
:GSICommandXXX()
{
	//errorType SetLength(GSIChar l);
    //errorType SetReturnedDataLength(GSIChar l);

	GSIRecord &r=GetGSIRecord();
	r.SetType(rtUser0);
	r.SetLength(0);
	r.SetSubfunctionString(GetCommandString());
}

STLCommandXXX::~STLCommandXXX()
{
}

/*!
STLCommand::FillGSIRecord()
*/
errorType STLCommandXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
	r.SetSubfunctionString(GetCommandString());
	wxFAIL_MSG("Called default STLCommandXXXFillGSIRecord()");
	return errNotImplemented;
}

/*!
errorType	STLCommandXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)	//execute the class specific command
Terminate the virtual function chain here
*/
errorType	STLCommandXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)	//execute the class specific command
{
	sr;
	Frame;
	wxFAIL_MSG("Called STLCommand::Execute with GSIFrame as parameter");
	return errNotImplemented;
}


//define our own Execute for StartleFrame specific functions
errorType	STLCommandXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)	//execute the class specific command
{
	sr;
	Frame;
	wxFAIL_MSG("Called STLCommand::Execute with STLFrame as parameter");
	return errNotImplemented;
}

/*!
errorType	STLCommandXXX::ReadFixedFields(wxSocketBase &sock)		//reads qflag, at_tick
*/
errorType	STLCommandXXX::ReadFixedFields(wxSocketBase &sock)		//reads qflag, at_tick
{
/*----------Read the STL QUEUED/IMMEDIATE Flag field--------------------*/
wxInt16 queuedFlag;
wxUint16 atTick;
wxString s;
errorType rv=errNone;

#if STL_DEBUG_USING_XXX

    sock.Read(&queuedFlag,sizeof (queuedFlag));
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
        return(sockErr.GetErrorCode());
	}

	s.Printf("flag=%i: ",queuedFlag);
    //GetStatusText()->AppendText(s);
	AppendText(s);
    if( (queuedFlag != STL_QUEUED) && (queuedFlag != STL_IMMEDIATE) )
    {
        GetStatusText()->AppendText("\n--- Flag error! ---\n");
        rv=errParameter;
    }
	else
	{
		if(queuedFlag==	STL_QUEUED)
			AppendText("Queued\n");
		else
			AppendText("Immediate\n");
	}
/*-----------------------------------------------------*/

/*--------------Read the atTick field----------------*/
    sock.Read(&atTick,sizeof(atTick));
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
        return(sockErr.GetErrorCode());
	}

	s.Printf("at tick=%i\n",atTick);
    AppendText(s);
//----Read all fixed record elements---------------------------------------------

	SetAtTick(atTick);
	SetQueuedFlag(queuedFlag);
#else
queuedFlag;
atTick;
s;
rv;
sock;

#endif
	return rv;
}

/*!
void STLCommandXXX::SetQueuedFlag(wxInt16 flag)
*/
void STLCommandXXX::SetQueuedFlag(wxInt16 flag)
{
	wxASSERT_MSG(flag==STL_IMMEDIATE || flag== STL_QUEUED,"Bad STLQueuedFlag!");

	if(flag == STL_IMMEDIATE || flag ==STL_QUEUED)
		m_queuedFlag=flag;
}

//--- End STLCommandXXX ---------------------



/***************************************************
	STLCommandSetFrequencyXXX ::STLCommandSetFrequencyXXX
****************************************************/
STLCommandSetFrequencyXXX::STLCommandSetFrequencyXXX(wxUint16 frequencyHz,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetNode(node);
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetCommandString("STLCommandSetFrequencyXXX");

	SetFrequency(frequencyHz);
	if(FillGSIRecord()== errNone)
		return;
	wxFAIL_MSG("STLCommandSetFrequency ctor");
}

/**************************************************************
	dtor
**************************************************************/
STLCommandSetFrequencyXXX::~STLCommandSetFrequencyXXX()
{
}

void STLCommandSetFrequencyXXX::SetFrequency(wxUint16 frequency)
{
	m_frequency=frequency;

	if(m_frequency<STLMinFrequency)
		m_frequency=STLMinFrequency;
	if(m_frequency>STLMaxFrequency)
		m_frequency=STLMaxFrequency;
//fout=(phase x clock) /2^32 (phase=32 bit user word, clock in MHz, fout in MHz)

	double f=m_frequency;
	f/=1000000;//convert Hz to MHz
	unsigned long deltaPhase;
	deltaPhase=(f*0xffffffff)/STL_DDS_CLOCK;
	m_deltaPhase=deltaPhase;


	m_b0=  (m_deltaPhase & 0x000000ff);
	m_b1=( (m_deltaPhase & 0x0000ff00) >>8 );
	m_b2=( (m_deltaPhase & 0x00ff0000) >>16);
	m_b3=( (m_deltaPhase & 0xff000000) >>24);

}


/**********************************************************************
errorType STLCommandSetFrequencyXXX::FillGSIRecord()

To set a frequency we must send 2 records.
This is because the startle pic code expects only one word of data
for every command.
So we send one rs232 record with 2 startle records in it.
The startle pic code will correctly place the two records into the
execution queue (for queued functions) or execute immediately for
immediate functions

# STL_COMMAND_SET_FREQUENCY
# STL_COMMAND_SET_FREQUENCY_UPPER
First send SET_FREQUENCY with lower 2 bytes of DDS word
then send  SET_FREQUENCY_UPPER with upper 2 bytes of DDS word
The pic will write the DDS word on receipt of SET_FREQUENCY_UPPER
**********************************************************************/
errorType STLCommandSetFrequencyXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();

//xxx pic only uses 2 bytes in its internal command q

	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_SET_FREQUENCY);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,m_b0);   //lsb
	r.SetData(STL_REC_OFFSET_P0_MSB,m_b1);  //msb

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

//Now send a 2nd record of type STL_COMMAND_SET-FREQUENCY_UPPER
//But append it to the first record. pic code knows how to handle this

	wxUint8 offset=STL_REC_OFFSET_P2_MSB;

	r.SetData(offset+STL_REC_OFFSET_TYPE,STL_COMMAND_SET_FREQUENCY_UPPER);
	r.SetData(offset+STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(offset+STL_REC_OFFSET_P0_LSB,m_b2);   //lsb
	r.SetData(offset+STL_REC_OFFSET_P0_MSB,m_b3);  //msb

    r.SetData(offset+STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(offset+STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

	r.SetLength(12);
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}


//-------- End STLCommandSetFrequencyXXX

/*!
*********************************************************
	ctor
	STLCommandSetAmplitudeSPLXXX ::STLCommandSetAmplitudeSPLXXX
***********************************************************/
STLCommandSetAmplitudeSPLXXX::STLCommandSetAmplitudeSPLXXX(wxInt16 dB,LUT &lut,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX(),m_LUT(lut)
{
	SetCommandString("STLSetAmplitudeSPL");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);

	if(dB<0) //-ve == mute

	if(dB<0)
		dB=-1;
	if(dB>STLMaxDB)
		dB=STLMaxDB;
	m_amplitudeSPL=dB;
	if(FillGSIRecord()== errNone)
		return;

	wxFAIL_MSG("STLCommandSetAmplitudeSPL ctor");
}

/**************************************************************
	dtor
**************************************************************/
STLCommandSetAmplitudeSPLXXX::~STLCommandSetAmplitudeSPLXXX()
{
}


/**********************************************************************
errorType STLCommandSetAmplitudeSPLXXX::FillGSIRecord()

**********************************************************************/
errorType STLCommandSetAmplitudeSPLXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
float dB=GetAmplitudeSPL();
wxUint8 dBByte;
//xxx pic only uses 2 bytes in its internal command q
/*
LM1972 uses .5dB steps up to 48dB attenuation, then 1dB steps up to 100dB
A value of 0=full attenuation (104dB)
*/

	if(dB<0)
		dB=0;
	if(dB>127)
		dB=127;
	//Use LUT to convert to LM1972 value required for this SPL
	dBByte=GetLUT().Item(dB);
	SetOutputByte(dBByte);
//Underlying GSIRecord uses SET_AMPLITUDE as SET_AMPLITUDE_SPL is not
//recognised by the m/c
	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_SET_AMPLITUDE);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,dBByte);   //lsb of word to write to LM1972
	r.SetData(STL_REC_OFFSET_P0_MSB,0);  //msb

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

	r.SetLength(6);
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}



//--- End STLCommandSetAmplitudeSPLXXX


/*!
*************************************************************************
	ctor
	STLCommandSetNoiseAmplitudeSPLXXX ::STLCommandSetNoiseAmplitudeSPLXXX
****************************************************************************/
STLCommandSetNoiseAmplitudeSPLXXX::STLCommandSetNoiseAmplitudeSPLXXX(wxInt16 dB,LUT &lut,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX(),m_LUT(lut)
{
	SetCommandString("STLSetNoiseAmplitudeSPLXXX");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);

	if(dB<0) //-ve == mute

	if(dB<0)
		dB=-1;
	if(dB>STLMaxDB)
		dB=STLMaxDB;
	m_amplitudeSPL=dB;
	if(FillGSIRecord()== errNone)
		return;

	wxFAIL_MSG("STLCommandSetNoiseAmplitudeSPL ctor");
}


/**************************************************************
	dtor
**************************************************************/
STLCommandSetNoiseAmplitudeSPLXXX::~STLCommandSetNoiseAmplitudeSPLXXX()
{
}


/**********************************************************************
errorType STLCommandSetNoiseAmplitudeSPLXXX::FillGSIRecord()

**********************************************************************/
errorType STLCommandSetNoiseAmplitudeSPLXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
float dB=GetAmplitudeSPL();
//wxUint8 dBByte;
//xxx pic only uses 2 bytes in its internal command q
/*
LM1972 uses .5dB steps up to 48dB attenuation, then 1dB steps up to 100dB
A value of 0=full attenuation (104dB)
*/

	if(dB<0)
		dB=0;
	if(dB>127)
		dB=127;
	//Use LUT to convert to LM1972 value required for this SPL
	//dBByte=GetLUT().Item(dB);

//Underlying GSIRecord uses SET_AMPLITUDE as SET_AMPLITUDE_SPL is not
//recognised by the m/c
	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_SET_NOISE_AMPLITUDE);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,dB);   //lsb of word to write to LM1972
	r.SetData(STL_REC_OFFSET_P0_MSB,0);  //msb

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

	r.SetLength(6);
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}

//--- End STLCommandSetNoiseAmplitudeSPLXXX


/***************************************************
	STLCommandResetXXX ::STLCommandResetXXX
****************************************************/
STLCommandResetXXX::STLCommandResetXXX(wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{

	SetNode(node);
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetCommandString("STLReset");
	FillGSIRecord();
}

/**************************************************************
	dtor
**************************************************************/
STLCommandResetXXX::~STLCommandResetXXX()
{
}


/**********************************************************************
errorType STLCommandReset::FillGSIRecord()
Simply send STL_REC_OFFSET_TYPE,STL_COMMAND_RESET
**********************************************************************/
errorType STLCommandResetXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();

	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_RESET);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

	r.SetLength(2);
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}


/***************************************************
	STLCommandSetChannelXXX ::STLCommandSetChannelXXX
****************************************************/
STLCommandSetChannelXXX::STLCommandSetChannelXXX(wxInt16 chan,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetCommandString("STLSetChannel");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);

	SetChannel(chan);

	if(FillGSIRecord()== errNone)
		return;
	wxFAIL_MSG("STLCommandSetChannelXXX ctor");
}

/**************************************************************
	dtor
**************************************************************/
STLCommandSetChannelXXX::~STLCommandSetChannelXXX()
{
}

/*!
STLCommandSetChannelXXX::SetChannel(wxInt16 chan)
*/
void STLCommandSetChannelXXX::SetChannel(wxInt16 chan)
{
	if(chan>STLMaxChan)
		chan=STLMaxChan;
	if(chan<STLMinChan)
		chan=STLMinChan;
	m_chan=chan;
}

/**********************************************************************
errorType STLCommandSetChannelXXX::FillGSIRecord()

**********************************************************************/
errorType STLCommandSetChannelXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();

	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_SET_CHANNEL);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,0);   //lsb
	r.SetData(STL_REC_OFFSET_P0_MSB,m_chan);	//msb offset of 3

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb


	r.SetLength(6);
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}

//------ End STLCommandSetChannelXXX

/*!
********************************************************************
	ctor
STLCommandSetAmplitudeLUTValueXXX::STLCommandSetAmplitudeLUTValueXXX
**********************************************************************/
STLCommandSetAmplitudeLUTValueXXX::STLCommandSetAmplitudeLUTValueXXX(wxInt16 index,wxInt16 value,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetCommandString("STLSetAmplitudeLUTValue");
	SetIndex(index);
	SetValue(value);
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);
	wxFAIL_MSG("Not implemented");
	if(FillGSIRecord()== errNone)
		return;


}

STLCommandSetAmplitudeLUTValueXXX::~STLCommandSetAmplitudeLUTValueXXX()
{
}

/*!
STLCommandSetAmplitudeLUTValueXXX::FillGSIRecord()
*/
errorType STLCommandSetAmplitudeLUTValueXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
	r.SetSubfunctionString(GetCommandString());
	wxFAIL;
	return errNotImplemented;
}

//---------End STLCommandSetAmplitudeLUTValueXXX ------------------


//xx
/***************************************************************************************************************
STLCommandOpenDeviceXXX(wxTextCtrl  *status_text=NULL,const wxString & dev_str="",wxUint16 at_tick=0,wxInt16 queued_flag=STL_IMMEDIATE, wxInt16 node=GSI_NODE_THIS)
Don't know how to utilise usb as yet, so will assume that the device will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present

This function will just fill in the handle and filename information for the device
The open will be performed by the GSIServer code, calling in to GSIFrame
The at_tick, queued, variables are not needed. Kept here for consistency
****************************************************************************************************************/
STLCommandOpenDeviceXXX::STLCommandOpenDeviceXXX(const wxString & dev_str,STLQueuedFlag queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
wxString s;
	SetAtTick(at_tick);
	SetQueuedFlag(queued_flag);
//Use the GSICommand version to fill in the details
	//GSICommandOpenDevice odCmd (dev_str);

	SetDevStr(dev_str);
	SetNode(node);
    m_handle=GetDeviceHandle();

}

STLCommandOpenDeviceXXX::~STLCommandOpenDeviceXXX()
{
}

errorType STLCommandOpenDeviceXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}


/***************************************************************************************************************
errorType STLCommandCloseDeviceXXX::STLCommandCloseDeviceXXX()
Don't know how to utilise usb as yet, so will assume that the decice will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present
****************************************************************************************************************/
STLCommandCloseDeviceXXX::STLCommandCloseDeviceXXX(GSIDevHandle handle,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node) //Client has sent OpenDevice command
:STLCommandXXX()
{
	SetNode(node);
	SetAtTick(at_tick);
	SetQueuedFlag(queued_flag);

	SetCommandString("STLCloseDeviceXXX");
	handle;

}


STLCommandCloseDeviceXXX::~STLCommandCloseDeviceXXX()
{
}

errorType STLCommandCloseDeviceXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}



/*!
errorType STLCommandCloseDeviceXXX::SetDeviceHandle(GSIDevHandle h)
*/
errorType STLCommandCloseDeviceXXX::SetDeviceHandle(GSIDevHandle h)
{
	wxASSERT_MSG(h >=0 && h< GSIMaxNumDeviceHandles,"Bad handle");
	if(h >=0 && h< GSIMaxNumDeviceHandles)
	{
		m_devHandle=h;
		return errNone;
	}

	m_devHandle=GSINotAValidDevHandle;
	return errBadHandle;
}

//-----End STLCloseDeviceXXX -------------------------


/*!
ctor
class STLCommandWaitForCompletionXXX : public STLCommand
class that holds hardware specific data.
A Node* or Device* can Write to a GSIDevice using this class.
This class holds a GSIRecord that can be read by the microcontrolelr firmware
*/

STLCommandWaitForCompletionXXX::STLCommandWaitForCompletionXXX(wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetCommandString("STLWaitForCompletionXXX");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);

	if(FillGSIRecord()== errNone)
		return;
	wxFAIL_MSG("STLWaitForCompletion ctor");

}

STLCommandWaitForCompletionXXX::~STLCommandWaitForCompletionXXX()
{
}

//Access functions
errorType		STLCommandWaitForCompletionXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
//Poll WAIT_FOR_COMPLETION till ok
	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_GET_STATUS);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,0);   //lsb
	r.SetData(STL_REC_OFFSET_P0_MSB,0);	//msb offset of 3

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

	r.SetLength(6);

	r.SetSubfunctionString(GetCommandString());
	return errNone;
}

//---- End STLCommandWaitForCompletionXXX ----------------


/*!
	ctor
class STLCommandGetStatusXXX :

class that holds hardware specific data.
A Node* or Device* can Write to a GSIDevice using this class.
This class holds a GSIRecord that can be read by the microcontrolelr firmware
*/

STLCommandGetStatusXXX::STLCommandGetStatusXXX(wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetCommandString("STLGetStatusXXX");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);

	if(FillGSIRecord()== errNone)
		return;
	wxFAIL_MSG("STLCommandGetStatus ctor");

}



STLCommandGetStatusXXX::~STLCommandGetStatusXXX()
{
}


errorType	STLCommandGetStatusXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();
	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_GET_STATUS);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,0);   //lsb
	r.SetData(STL_REC_OFFSET_P0_MSB,0);	//msb offset of 3

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

	r.SetLength(6);

	r.SetSubfunctionString(GetCommandString());
	return errNone;
}

//---End STLCommandGetStatusXXX---------------------------


/*!********************************************
	ctor
	STLCommandAcquireXXX ::STLCommandAcquireXXX
****************************************************/
STLCommandAcquireXXX::STLCommandAcquireXXX(wxUint16 num_data_points,wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetCommandString("STLAcquireXXX");
	SetNumDataPoints(num_data_points);
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);

	if(FillGSIRecord()== errNone)
		return;
	wxFAIL_MSG("STLCommandAcquire ctor");
}

/**************************************************************
	dtor
**************************************************************/
STLCommandAcquireXXX::~STLCommandAcquireXXX()
{
}


/**********************************************************************
errorType STLCommandAcquireXXX::FillGSIRecord()

**********************************************************************/
errorType STLCommandAcquireXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();

	if(GetNumDataPoints() > STLMaxADCArraySize)
		m_numDataPoints=STLMaxADCArraySize;

	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_ACQUIRE);
	r.SetData(STL_REC_OFFSET_FLAG,GetQueuedFlag());

    r.SetData(STL_REC_OFFSET_P0_LSB,(m_numDataPoints &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P0_MSB,(m_numDataPoints >>8));	//msb offset of 3

    r.SetData(STL_REC_OFFSET_P1_LSB,(GSIChar) (GetAtTick() &0x00ff));   //lsb
	r.SetData(STL_REC_OFFSET_P1_MSB,(GSIChar) (GetAtTick() >> 8));       //msb

	r.SetLength(6);

	r.SetSubfunctionString(GetCommandString());
	return errNone;
}
//------End STLCommandAcquireXXX-------------------------




/*!********************************************
	ctor
	STLCommandExecuteXXX ::STLCommandExecuteXXX
****************************************************/
STLCommandExecuteXXX::STLCommandExecuteXXX(wxInt16 queued_flag,wxUint16 at_tick, wxInt16 node)
:STLCommandXXX()
{
	SetCommandString("STLCommandExecuteXXX");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetNode(node);
	if(FillGSIRecord()== errNone)
		return;
	wxFAIL_MSG("STLCommandExecute ctor");
}

/**************************************************************
	dtor
**************************************************************/
STLCommandExecuteXXX::~STLCommandExecuteXXX()
{
}


/**********************************************************************
errorType STLCommandExecute::FillGSIRecord()

**********************************************************************/
errorType STLCommandExecuteXXX::FillGSIRecord()
{
GSIRecord &r=GetGSIRecord();


	r.SetData(STL_REC_OFFSET_TYPE,STL_COMMAND_EXECUTE);
	r.SetData(STL_REC_OFFSET_FLAG,STL_IMMEDIATE);		//always immediate

	r.SetLength(2);
	r.SetSubfunctionString(GetCommandString());
	return errNone;
}



#endif //STARTLE_SERVER
