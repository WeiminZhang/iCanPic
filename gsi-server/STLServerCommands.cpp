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
#  pragma implementation "server.cpp"
#  pragma interface "server.cpp"
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

#include "wx/socket.h"
#include "include/gsi.hpp"
#include "include/socket.hpp"
#include "include/server.hpp"

#ifdef STARTLE_SERVER
#include "startle.hpp"
#include "STLServerCommands.hpp"
#include "microcontroller.h"

#include "StartleFrame.hpp"

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


STLServerCommandXXX::STLServerCommandXXX(wxSocketBase & sock)
//:STLCommandXXX()
:m_Sock(sock)
{
	Init();
}

STLServerCommandXXX::~STLServerCommandXXX()
{

}

void STLServerCommandXXX::Init()
{
	SetReadFlag(false);

}


/*!
void STLServerCommand::Read()
Provide this default Read so that commands that do not have any data
do not need to implement a Read() of their own
*/
errorType STLServerCommandXXX::Read()
{
errorType rv=errNotImplemented;
	//=ReadFixedFields(GetSock());		//reads qflag, at_tick

	wxFAIL_MSG("Called base STLServerCommand::Read()");
	return rv;
}



#warning ("This function redundant?? Uses verison in base class??")
/*!
errorType STLServerCommandXXX::ReadFixedFields()
Read:
qflag
at_tick
*/
errorType STLServerCommandXXX::ReadFixedFields(StartleRecord &r)
{
/*----------Read the STL QUEUED/IMMEDIATE Flag field--------------------*/
wxInt16 queuedFlag;
wxUint16 atTick;
wxString s;
errorType rv=errNone;

#if STL_DEBUG_USING_XXX
queuedFlag;
atTick;
s;
rv;
r;
#else
	wxString s1="";
    GetSock().Read(&queuedFlag,sizeof (queuedFlag));
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorCode());
	}
	s1.Printf("%i ",queuedFlag);

	s.Printf("flag=%i: ",queuedFlag);
    GetStatusText()->AppendText(s);

    if( (queuedFlag != STL_QUEUED) && (queuedFlag != STL_IMMEDIATE) )
    {
        GetStatusText()->AppendText("\n--- Flag error! ---\n");
        rv=errParameter;
    }
	else
	{
		if(queuedFlag==	STL_QUEUED)
			GetStatusText()->AppendText("Queued\n");
		else
			GetStatusText()->AppendText("Immediate\n");
	}
/*-----------------------------------------------------*/

/*--------------Read the atTick field----------------*/
    GetSock().Read(&atTick,sizeof(atTick));
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorCode());
	}
	s1=s1+atTick+" ";

	s.Printf("at tick=%i \n",atTick);
    GetStatusText()->AppendText(s);
//----Read all fixed record elements---------------------------------------------

	SetAtTick(atTick);
	SetQueuedFlag(static_cast<STLQueuedFlag>(queuedFlag));
	AppendToReadFromSocketString(s1);
#endif
	return rv;

}



//---- End STLServerCommandXXX ---------------




/*!
STLServerCommandSetAmplitudeSPLXXX ::STLServerCommandSetAmplitudeSPLXXX
*/
/***************************************************
	STLServerCommandSetAmplitudeSPLXXX ::STLServerCommandSetAmplitudeSPLXXX
****************************************************/


#if GSI_USE_SHARED_POINTERS
STLServerCommandSetAmplitudeSPLXXX::STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock,boost::shared_ptr<StartleLUT> lut)
#else
STLServerCommandSetAmplitudeSPLXXX::STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock,StartleLUT * const lut)
#endif
:STLCommandSetAmplitudeSPLXXX(0,lut),
GSISocketXXX(sock)
{
	SetCommandString("STLCommandSetAmplitudeSPLXXX");
}

/**************************************************************
	dtor
**************************************************************/
STLServerCommandSetAmplitudeSPLXXX::~STLServerCommandSetAmplitudeSPLXXX()
{
}


/*************************************************************************
    errorType STLServerCommandSetAmplitudeSPL::Read(wxSocketBase s&ock)
will have already read the Socket command type and the command id
now need to read the command specific data from the client.
Should only be called from a GSISocket server or derived class

Note that the corresponding ::Write function will be waiting for a
ServerReturnRecord. This function does not return this record as it
doesn't know if the task was completed without error.
The calling code will need to perform the required hardware (or other) tasks
and return the ServerReturnRecord indicating status of the function

*************************************************************************/
errorType STLServerCommandSetAmplitudeSPLXXX::Read(wxSocketBase & sock)
{
//read amplitude
	wxInt16 amplitude;
errorType rv;

	rv=ReadFixedFields(GetSock());		//reads qflag, at_tick
	sock.Read(&amplitude,sizeof(amplitude));
	SetReadFlag(true);
	AppendToReadFromSocketString(amplitude);
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
		SetLastError(sockErr.GetErrorType());
        return(sockErr.GetErrorType());
	}
	SetAmplitude(amplitude);
	FillGSIRecord();				//perform LUT conversion
    return errNone;
}

/*!
errorType STLServerCommandSetAmplitudeSPLXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandSetAmplitudeSPLXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.SetAmplitudeSPLXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);


	return rv;
}

//--- End STLServerCommandSetAmplitudeSPLXXX


/*!
STLServerCommandSetNoiseAmplitudeSPLXXX ::STLServerCommandSetNoiseAmplitudeSPLXXX
*/
/***************************************************
	STLServerCommandSetNoiseAmplitudeSPLXXX ::STLServerCommandSetNoiseAmplitudeSPLXXX
****************************************************/

#if GSI_USE_SHARED_POINTERS
STLServerCommandSetNoiseAmplitudeSPLXXX::STLServerCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,boost::shared_ptr<StartleLUT> lut)
#else
STLServerCommandSetNoiseAmplitudeSPLXXX::STLServerCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,StartleLUT * const lut)
#endif
:STLCommandSetNoiseAmplitudeSPLXXX(0,lut),
GSISocketXXX(sock)
{
	SetCommandString("STLCommandSetNoiseAmplitudeSPLXXX");
}

/**************************************************************
	dtor
**************************************************************/
STLServerCommandSetNoiseAmplitudeSPLXXX::~STLServerCommandSetNoiseAmplitudeSPLXXX()
{
}


/*************************************************************************
    errorType STLServerCommandSetNoiseAmplitudeSPL::Read(wxSocketBase s&ock)
will have already read the Socket command type and the command id
now need to read the command specific data from the client.
Should only be called from a GSISocket server or derived class

Note that the corresponding ::Write function will be waiting for a
ServerReturnRecord. This function does not return this record as it
doesn't know if the task was completed without error.
The calling code will need to perform the required hardware (or other) tasks
and return the ServerReturnRecord indicating status of the function

*************************************************************************/
errorType STLServerCommandSetNoiseAmplitudeSPLXXX::Read(wxSocketBase & sock)
{
//read amplitude
	wxInt16 amplitude;
	errorType rv;

	rv=ReadFixedFields(GetSock());		//reads qflag, at_tick
	sock.Read(&amplitude,sizeof(amplitude));
	SetReadFlag(true);
	AppendToReadFromSocketString(amplitude);

    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
		SetLastError(sockErr.GetErrorType());
        return(sockErr.GetErrorType());
	}
	SetAmplitude(amplitude);
	FillGSIRecord();				//perform LUT conversion
    return errNone;
}

/*!
errorType STLServerCommandSetNoiseAmplitudeSPLXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandSetNoiseAmplitudeSPLXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.SetNoiseAmplitudeSPLXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);


	return rv;
}

//--- End STLServerCommandSetAmplitudeSPLXXX


/*!
STLServerCommandSetChannelXXX ::STLServerCommandSetChannelXXX
*/
/***************************************************
	STLServerCommandSetChannelXXX ::STLServerCommandSetChannelXXX
****************************************************/
STLServerCommandSetChannelXXX::STLServerCommandSetChannelXXX(wxSocketBase & sock)
:STLCommandSetChannelXXX(),
GSISocketXXX(sock)
{
	SetCommandString("STLCommandSetChannelXXX");
}

/*!
***********************************************************
		dtor
STLServerCommandSetChannelXXX::~STLServerCommandSetChannelXXX()

***********************************************************/
STLServerCommandSetChannelXXX::~STLServerCommandSetChannelXXX()
{
}


/*!
	errorType STLServerCommandSetChannelXXX::Read()

will have already read the Socket command type and the command id
now need to read the command specific data from the client.
Should only be called from a GSISocket server or derived class

Note that the corresponding ::Write function will be waiting for a
ServerReturnRecord. This function does not return this record as it
doesn't know if the task was completed without error.
The calling code will need to perform the required hardware (or other) tasks
and return the ServerReturnRecord indicating status of the function


Reads
wxInt16 queuedFlag
wxUint16 atTick
wxInt16 channel
*************************************************************************/
errorType STLServerCommandSetChannelXXX::Read(wxSocketBase &sock)
{
//read amplitude
wxInt16 chan;
errorType rv;

	rv=ReadFixedFields(sock);		//reads qflag, at_tick

	sock.Read(&chan,sizeof(chan));
	SetReadFlag(true);
	AppendToReadFromSocketString(chan);
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
		SetLastError(sockErr.GetErrorType());
        return(sockErr.GetErrorType());
	}
	SetChannel(chan);
	FillGSIRecord();
	wxString s;
	s.Printf("Read SetChannel (%i) from client\n",chan);
	AppendText(s);
    return errNone;
}

/*!
errorType STLServerCommandSetChannelXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
*/
errorType STLServerCommandSetChannelXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.SetChannelXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);
	return rv;
}



/*!
STLServerCommandAcquire ::STLServerAcquire
*/
/***************************************************
	STLServerCommandAcquireXXX ::STLServerCommandAcquireXXX
****************************************************/
STLServerCommandAcquireXXX::STLServerCommandAcquireXXX(wxSocketBase & sock)
:STLCommandAcquireXXX(),	//use default parameters as socket will read
GSISocketXXX(sock)
{

	SetCommandString("STLCommandAcquireXXX");
}

/*!
***********************************************************
		dtor
STLServerCommandAcquireXXX::~STLServerCommandAcquireXXX()

***********************************************************/
STLServerCommandAcquireXXX::~STLServerCommandAcquireXXX()
{
}


/*!
	errorType STLServerCommandAcquireXXX::Read()

will have already read the Socket command type and the command id
now need to read the command specific data from the client.
Should only be called from a GSISocket server or derived class

Note that the corresponding ::Write function will be waiting for a
ServerReturnRecord. This function does not return this record as it
doesn't know if the task was completed without error.
The calling code will need to perform the required hardware (or other) tasks
and return the ServerReturnRecord indicating status of the function


Reads
wxInt16 queuedFlag
wxUint16 atTick
wxInt16 channel
*************************************************************************/
errorType STLServerCommandAcquireXXX::Read(wxSocketBase &sock)
{
//read amplitude
wxUint16 numDataPoints;
errorType rv;

	rv=ReadFixedFields(GetSock());		//reads qflag, at_tick
	sock.Read(&numDataPoints,sizeof(numDataPoints));
	SetReadFlag(true);
	AppendToReadFromSocketString(numDataPoints);

    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
		SetLastError(sockErr.GetErrorType());
        return(sockErr.GetErrorType());
	}
	SetNumDataPoints(numDataPoints);
	FillGSIRecord();
	AppendText("Read STLCommandAcquireXXX from client\n");
    return errNone;
}
/*!
errorType STLServerCommandAcquireXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandAcquireXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.AcquireXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);


	return rv;
}

//----End STLServerCommandAcquireXXX-----------



/***************************************************************
	STLServerCommandExecute ::STLServerCommandExecute
****************************************************************/
STLServerCommandExecuteXXX::STLServerCommandExecuteXXX(wxSocketBase & sock)
:STLCommandExecuteXXX(),
GSISocketXXX(sock)
{
	SetCommandString("STLCommandExecuteXXX");

}

/**************************************************************
	dtor
**************************************************************/
STLServerCommandExecuteXXX::~STLServerCommandExecuteXXX()
{
}

/*************************************************************************
    errorType STLServerCommandExecute::Read()
will have already read the Socket command type and the command id
now need to read the command specific data from the client.
Should only be called from a GSISocket server or derived class

Note that the corresponding ::Write function will be waiting for a
ServerReturnRecord. This function does not return this record as it
doesn't know if the task was completed without error.
The calling code will need to perform the required hardware (or other) tasks
and return the ServerReturnRecord indicating status of the function

Reads:
nothing to read

*************************************************************************/
errorType STLServerCommandExecuteXXX::Read(wxSocketBase &sock)
{
	errorType rv;
    rv=ReadFixedFields(sock);		//reads qflag, at_tick
	FillGSIRecord();			//fill in the Hdw record
	AppendText("Read Execute from client\n");
    return errNone;
}

/*!
errorType STLServerCommandExecuteXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
*/
errorType STLServerCommandExecuteXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;

	rv=Frame.ExecuteXXX(*this);
	if(rv)
	{
		sr.SetReturnCode(rv);
		return rv;
	}

	sr.ResizeDataBuff(Frame.GetADCNumRead());
	sr.SetNumInts(Frame.GetADCNumRead());
	wxInt16 *src=Frame.GetADCBuffer();
	wxInt16 *dst=sr.GetDataArray();
	for(int i=0;i<Frame.GetADCNumRead();i++)
	{
		dst[i]=src[i];
	}
	Frame.SetADCNumRead(0);
	sr.SetReturnCode(GSISocketReturnData::GSI_SUCCESS);

	//Now perform a WaitForCompletion
	//errorType StartleFrame::ResetXXX(STLCommandResetXXX & cmd)
	//errorType	WaitForCompletionXXX(STLCommandWaitForCompletionXXX & cmd);

	//This sometimes fails if we are setting an amplitude as the last executable command. A likely occurence, so we need to
	//place a little delay in here
	wxStopWatch sw;
	do
	{

	}while(sw.Time() <1000);		//wait 1000mS for everything to complete, might even need mc to send us a record telling us it is ready


	STLCommandWaitForCompletionXXX cmd;

	rv=Frame.WaitForCompletionXXX(cmd);

	return rv;
}


//---End STLServerCommandExecuteXXX-------------



/*!
*****************************************************************************
	STLServerCommandWaitForCompletionXXX ::STLServerCommandWaitForCompletionXXX
****************************************************************************/
STLServerCommandWaitForCompletionXXX::STLServerCommandWaitForCompletionXXX(wxSocketBase & sock)
:STLCommandWaitForCompletionXXX(),
GSISocketXXX(sock)
{
	SetCommandString("STLWaitForCompletionXXX");
}

/*!
*************************************************************
	dtor
**************************************************************/
STLServerCommandWaitForCompletionXXX::~STLServerCommandWaitForCompletionXXX()
{
}

/*************************************************************************
    errorType STLServerCommandWaitForCompletionXXX::Read()
will have already read the Socket command type and the command id
now need to read the command specific data from the client.
Should only be called from a GSISocket server or derived class

Note that the corresponding ::Write function will be waiting for a
ServerReturnRecord. This function does not return this record as it
doesn't know if the task was completed without error.
The calling code will need to perform the required hardware (or other) tasks
and return the ServerReturnRecord indicating status of the function

Reads:
nothing to read

*************************************************************************/
errorType STLServerCommandWaitForCompletionXXX::Read(wxSocketBase &sock)
{
errorType rv;
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	FillGSIRecord();			//fill in the Hdw record
	AppendText("Read WaitForCompletion from client\n");
    return errNone;
}


/*!
errorType STLServerCommandWaitForCompletionXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandWaitForCompletionXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.WaitForCompletionXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);


	return rv;
}


//---------End STLServerCommandWaitForCompletionXXX-------------



/*!
    errorType STLServerCommandOpenDeviceXXX::STLServerCommandOpenDeviceXXX(const wxString & device)
		ctor
Read the device name from the socket client
# wxInt16 lengthOfSstring
# char buff[lengthOfString]

*/
STLServerCommandOpenDeviceXXX::STLServerCommandOpenDeviceXXX(wxSocketBase &sock)
//:STLServerCommandXXX(sock)
:GSISocketXXX(sock)
{
	SetCommandString("STLServerCommandOpenDeviceXXX");
}


/*!
GSIServerCommandOpenDeviceXXX::~GSIServerCommandOpenDeviceXXX()
*/
STLServerCommandOpenDeviceXXX::~STLServerCommandOpenDeviceXXX()
{
}

/**********************************************************************************************
    errorType STLServerCommandOpenDevice::Read(const wxString &m_device)
;will have already read the Socket command type and the command id
A polymorphic Read()
***********************************************************************************************/
errorType STLServerCommandOpenDeviceXXX::Read(wxSocketBase &sock)
{
wxInt16 length;
errorType rv;

    //sock.SetFlags(wxSOCKET_WAITALL);
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	if(rv)
		return rv;

//reading a string, so first read its length, followed by the buffer
//----read the data length in bytes-----------------       //socket server will understand this format
    GetSock().Read(&length,sizeof(wxInt16));
        if(GetSock().Error())
            return(errFail);
	AppendToReadFromSocketString(length);
//-----Read the data, a string in this instance ----
    char *buff=DEBUG_NEW char[length];
    GetSock().Read(buff,length);   //Read the str
    m_DeviceStr=wxString(buff, length);
	AppendToReadFromSocketString(m_DeviceStr);
    delete buff;
    if(GetSock().Error())
        return(errFail);
    SetReadFlag(true);
    return errNone;
}


/*!
errorType STLServerCommandOpenDeviceXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandOpenDeviceXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
GSIDevHandle hDev;
errorType rv;

	wxInt16 * const array=sr.GetDataArray();

	array[0]=GSINotAValidDevHandle;		//assume failure
	rv=Frame.OpenDevice(GetDeviceString(),&hDev);
	if(rv==errNone)
	{
		SetDeviceHandle(hDev);
		array[0]=GetDeviceHandle();	 //return the handle
		sr.SetNumInts(1);

		rv=Frame.CreateNode(GetDeviceHandle()); //Using default node of GSI_NODE_THIS

		if (rv)
		{
			sr.SetReturnCode(GSISocketReturnData::GSI_NOT_FOUND);//
			rv=Frame.CloseDevice(GetDeviceHandle());

		}
		else
		{
			sr.SetNumInts(2);
			array[1]=GSI_NODE_THIS;
		}
	}

	return rv;
}

//------End STLServerCommandOpenDeviceXXX----


/*!
    errorType STLServerCommandCloseDeviceXXX::STLServerCommandOpenDeviceXXX(const wxString & device)
		ctor
Read the device name from the socket client
# wxInt16 lengthOfSstring
# char buff[lengthOfString]

*/
STLServerCommandCloseDeviceXXX::STLServerCommandCloseDeviceXXX(wxSocketBase &sock)
//:STLServerCommandXXX(sock)
:GSISocketXXX(sock)
{
	SetCommandString("STLServerCommandCloseDeviceXXX");
}


/*!
GSIServerCommandOpenDeviceXXX::~GSIServerCommandOpenDeviceXXX()
*/
STLServerCommandCloseDeviceXXX::~STLServerCommandCloseDeviceXXX()
{
}



/*!
errorType STLServerCommandCloseDeviceXXX::Execute(StartleFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandCloseDeviceXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
GSIDevHandle hDev;
errorType rv;
	sr;
	hDev;
	rv=Frame.CloseDevice(GetDeviceHandle());
	if(rv)
		sr.SetReturnCode(rv);
	return rv;
}


/**********************************************************************************************
    errorType STLServerCommandCloseDeviceXXX::Read(wxSocketBase &sock)
;will have already read the Socket command type and the command id
A polymorphic Read()
***********************************************************************************************/

errorType STLServerCommandCloseDeviceXXX::Read(wxSocketBase &sock)
{
GSIDevHandle hDev;
errorType rv;

    sock.SetFlags(wxSOCKET_WAITALL);
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	if(rv)
		return rv;

//reading a string, so first read its length, followed by the buffer
//----read the data length in bytes-----------------       //socket server will understand this format
    GetSock().Read(&hDev,sizeof(hDev));
    if(GetSock().Error())
	    return(errFail);
	AppendToReadFromSocketString(hDev);
	SetDeviceHandle(hDev);
    SetReadFlag(true);
    return errNone;
}


//------End STLServerCommandCloseDeviceXXX----


/*!
    errorType STLServerCommandSetFrequencyXXX::STLServerCommandSetFrequencyXXX(const wxString & device)
		ctor
Read the frequecy in Hz from the socket client

*/
STLServerCommandSetFrequencyXXX::STLServerCommandSetFrequencyXXX(wxSocketBase &sock)
:GSISocketXXX(sock)
{
	SetCommandString("STLServerCommandSetFrequencyXXX");
}


/*!
GSIServerCommandSetFrequencyXXX::~GSIServerCommandSetFrequencyXXX()
*/
STLServerCommandSetFrequencyXXX::~STLServerCommandSetFrequencyXXX()
{
}

/**********************************************************************************************
    errorType STLServerCommandSetFrequency::Read(const wxString &m_device)
;will have already read the Socket command type and the command id
A polymorphic Read()
We take a sock as a parameter even though this code has access to the sock
This is because it is called from a base class and some of the polymorphic
classes derived from the base will not have a sock. This overrides that Read(sock)
***********************************************************************************************/
errorType STLServerCommandSetFrequencyXXX::Read(wxSocketBase &sock)
{
errorType rv;
wxUint16 f;

    sock.SetFlags(wxSOCKET_WAITALL);
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	if(rv)
		return rv;


//-----Read the data, a wxUint16 in this instance ----

    sock.Read(&f,sizeof(f));   //Read the str
    if(sock.Error())
        return(errFail);
	AppendToReadFromSocketString(f);
	SetFrequency(f);
	FillGSIRecord();

    SetReadFlag(true);
    return errNone;
}


/*!
errorType STLServerCommandSetFrequencyXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandSetFrequencyXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.SetFrequencyXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);


	return rv;
}


//----- End STLServerCommandSetFrequencyXXX------------



/*!
    errorType STLServerCommandResetyXXX::STLServerCommandResetXXX
		ctor
*/
STLServerCommandResetXXX::STLServerCommandResetXXX(wxSocketBase &sock)
:GSISocketXXX(sock)
{
	SetCommandString("STLServerCommandResetXXX");
}


/*!
GSIServerCommandSetFrequencyXXX::~GSIServerCommandSetFrequencyXXX()
*/
STLServerCommandResetXXX::~STLServerCommandResetXXX()
{
}

/**********************************************************************************************
    errorType STLServerCommandReset::Read(wxSocketBase sock)
;will have already read the Socket command type and the command id
A polymorphic Read()
We take a sock as a parameter even though this code has access to the sock
This is because it is called from a base class and some of the polymorphic
classes derived from the base will not have a sock. This overrides that Read(sock)
***********************************************************************************************/
errorType STLServerCommandResetXXX::Read(wxSocketBase &sock)
{
errorType rv;

    sock.SetFlags(wxSOCKET_WAITALL);
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	if(rv)
		return rv;

    FillGSIRecord();
    SetReadFlag(true);
    return errNone;
}


/*!
errorType STLServerCommandResetXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandResetXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;


	rv=Frame.ResetXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);


	return rv;
}


//----- End STLServerCommandReset------------


/*!
    errorType STLServerCommandSetAmplitudeLUTValueXXX::STLServerCommandSetAmplitudeLUTValueXXX(const wxString & device)
		ctor
*/
STLServerCommandSetAmplitudeLUTValueXXX::STLServerCommandSetAmplitudeLUTValueXXX(wxSocketBase &sock)
:STLCommandSetAmplitudeLUTValueXXX(),
GSISocketXXX(sock)
{
	SetCommandString("STLServerCommandSetAmplitudeLUTValueXXX");
}


/*!
GSIServerCommandSetAmplitudeLUTValueXXX::~GSIServerCommandSetAmplitudeLUTValueXXX()
*/
STLServerCommandSetAmplitudeLUTValueXXX::~STLServerCommandSetAmplitudeLUTValueXXX()
{
}

/**********************************************************************************************
    errorType STLServerCommandSetAmplitudeLUTValue::Read(const wxString &m_device)
;will have already read the Socket command type and the command id
A polymorphic Read()
We take a sock as a parameter even though this code has access to the sock
This is because it is called from a base class and some of the polymorphic
classes derived from the base will not have a sock. This overrides that Read(sock)
***********************************************************************************************/
errorType STLServerCommandSetAmplitudeLUTValueXXX::Read(wxSocketBase &sock)
{
errorType rv;
wxInt16 index;
wxInt16 value;

    sock.SetFlags(wxSOCKET_WAITALL);
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	if(rv)
		return rv;


//-----Read the index, a wxUint16 in this instance ----

    sock.Read(&index,sizeof(index));   //Read the index
    if(sock.Error())
        return(errFail);
	AppendToReadFromSocketString(index);
//-----Read the value

    sock.Read(&value,sizeof(value));   //Read the value
    if(sock.Error())
        return(errFail);
	AppendToReadFromSocketString(value);

	SetValue(value);
	SetIndex(index);

    SetReadFlag(true);
    return errNone;
}


/*!
errorType STLServerCommandSetLUTValueXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandSetAmplitudeLUTValueXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;

	rv=Frame.SetAmplitudeLUTValueXXX(*this);
	//rv=Frame.SetLUTValueXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);
	else
	{
		wxString s;
		s.Printf("Amplitude LUT(%i)=%i\n",GetIndex(),GetValue());
		AppendText(s);

	}

	return rv;
}


//----- End STLServerCommandSetAmplitudeLUTValueXXX------------




/*!
    errorType STLServerCommandSetNoiseAmplitudeLUTValueXXX::STLServerCommandSetNoiseAmplitudeLUTValueXXX(const wxString & device)
		ctor

*/
STLServerCommandSetNoiseAmplitudeLUTValueXXX::STLServerCommandSetNoiseAmplitudeLUTValueXXX(wxSocketBase &sock)
:STLCommandSetNoiseAmplitudeLUTValueXXX(),
GSISocketXXX(sock)
{
	SetCommandString("STLServerCommandSetNoiseAmplitudeLUTValueXXX");
}


/*!
GSIServerCommandSetNoiseAmplitudeLUTValueXXX::~GSIServerCommandSetNoiseAmplitudeLUTValueXXX()
*/
STLServerCommandSetNoiseAmplitudeLUTValueXXX::~STLServerCommandSetNoiseAmplitudeLUTValueXXX()
{
}

/**********************************************************************************************
    errorType STLServerCommandSetNoiseAmplitudeLUTValue::Read(const wxString &m_device)
;will have already read the Socket command type and the command id
A polymorphic Read()
We take a sock as a parameter even though this code has access to the sock
This is because it is called from a base class and some of the polymorphic
classes derived from the base will not have a sock. This overrides that Read(sock)
***********************************************************************************************/
errorType STLServerCommandSetNoiseAmplitudeLUTValueXXX::Read(wxSocketBase &sock)
{
errorType rv;
wxInt16 index;
wxInt16 value;

    sock.SetFlags(wxSOCKET_WAITALL);
	rv=ReadFixedFields(sock);		//reads qflag, at_tick
	if(rv)
		return rv;


//-----Read the index, a wxUint16 in this instance ----

    sock.Read(&index,sizeof(index));   //Read the index
    if(sock.Error())
        return(errFail);
	AppendToReadFromSocketString(index);
//-----Read the value

    sock.Read(&value,sizeof(value));   //Read the value
    if(sock.Error())
        return(errFail);
	AppendToReadFromSocketString(value);

	SetValue(value);
	SetIndex(index);

    SetReadFlag(true);
    return errNone;
}


/*!
errorType STLServerCommandSetLUTValueXXX::Execute(GSIFrame &Frame,GSISocketReturnData sr)	//execute the class specific command
Polymorphic function, currently called from GSICommandXXX
Probably need to make it called from STLCommand so that have access to
StartleFrame rather than GSIFrame
*/

errorType STLServerCommandSetNoiseAmplitudeLUTValueXXX::Execute(StartleFrame &Frame,GSISocketReturnData &sr)
{
errorType rv=errNone;

	rv=Frame.SetNoiseAmplitudeLUTValueXXX(*this);
	//rv=Frame.SetLUTValueXXX(*this);
	if(rv)
		sr.SetReturnCode(rv);
	else
	{
		wxString s;
		s.Printf("Noise amplitude LUT(%i)=%i\n",GetIndex(),GetValue());
		AppendText(s);

	}


	return rv;
}


//----- End STLServerCommandSetNoiseAmplitudeLUTValueXXX------------

#endif //#if STARTLE

