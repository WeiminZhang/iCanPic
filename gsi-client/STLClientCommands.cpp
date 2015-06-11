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
#  pragma implementation "GSICommands.cpp"
#  pragma interface "GSICommands.cpp"
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


//#include "gsi-common/include/gsi.hpp"

#ifdef STARTLE_SERVER


#include "..\\include\microcontroller.h"		//required for rtUser0 record type. This include file needs renaming
							//or should be incorporated in record.hpp
#include "..\\include\startle.hpp"
#include "STLClientCommands.hpp"



#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif



/*********************************************************************************************************************************************
STLClientCommand::STLClientCommandXXX(wxSocketBase & sock,)
	ctor
**********************************************************************************************************************************************/
STLClientCommandXXX::STLClientCommandXXX(wxSocketBase & sock)
:GSIClientCommandXXX(sock)
//:GSIClientCommandXXX(sock)
{
#warning message("socket client timeout set to 1000 seconds")
	sock.SetTimeout(1000);
}

STLClientCommandXXX::~STLClientCommandXXX()
{
}

void STLClientCommandXXX::Init()
{
	//SetQueuedFlag(STL_IMMEDIATE);
	//SetAtTick(0);
}


//---End STLClientCommandXXX -------------


/*!*
STLCommandSetFrequencyXXX ::STLCommandSetFrequencyXXX
	ctor
*/

STLClientCommandSetFrequencyXXX::STLClientCommandSetFrequencyXXX(wxSocketBase & sock,wxInt16 f,STLQueuedFlag queued_flag,STLAtTickParam at_tick,wxTextCtrl *const status_text)
:STLCommandSetFrequencyXXX(f,queued_flag,at_tick),
STLClientCommandXXX(sock)
{
errorType rv=errNone;
	SetStatusText(status_text);
	SetFrequency(f);


    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSIClientCommandSetFrequency ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

//dtor
STLClientCommandSetFrequencyXXX::~STLClientCommandSetFrequencyXXX()
{

}
/*!
errorType STLClientCommandSetFrequencyXXX::Write(bool use_written_flag)
*/
errorType STLClientCommandSetFrequencyXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");
    //GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending STL SetFrequency record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_SET_FREQUENCY;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------


//-----write the data, a wxUInt16 in this instance ----
	buff=GetFrequency();
    GetSock().Write(&buff,sizeof(buff));  //write the frequency
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandSetFrequency",errSocket);
    }

    return errNone;
}

//---End STLClientCommandSetFrequencyXXX





/*!
******************************************************
STLCommandSetAmplitudeSPLXXX ::STLCommandSetAmplitudeSPLXXX
	ctor
******************************************************/
STLClientCommandSetAmplitudeSPLXXX::STLClientCommandSetAmplitudeSPLXXX(wxSocketBase & sock,wxInt16 dB,StartleLUT * const lut,STLQueuedFlag queued_flag,wxUint16 at_tick,wxTextCtrl *const status_text)
:STLCommandSetAmplitudeSPLXXX(dB,lut,queued_flag,at_tick),
STLClientCommandXXX(sock)
{
errorType rv=errNone;
	SetStatusText(status_text);
    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSIClientCommandSetAmplitudeSPLXXX ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

STLClientCommandSetAmplitudeSPLXXX::~STLClientCommandSetAmplitudeSPLXXX()
{

}


errorType STLClientCommandSetAmplitudeSPLXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");
//#warning ("remove following line. Need to use default 10 sec timeout")
	//GetSock().SetTimeout(1000);		//debugging
    //GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending STL SetAmplitudeSPLXXX record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_SET_AMPLITUDE_SPL;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------


//-----write the data ----
	buff=GetAmplitudeSPL();
    GetSock().Write(&buff,sizeof(buff));  //write the data
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandSetAmplitudeSPL",errSocket);
    }

    return errNone;
}

//--- End SetClientCommandAmplitudeSPLXXX --------



/*!
*********************************************************************
STLCommandSetNoiseAmplitudeSPLXXX ::STLCommandSetNoiseAmplitudeSPLXXX
	ctor
**********************************************************************/
STLClientCommandSetNoiseAmplitudeSPLXXX::STLClientCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,wxInt16 dB,StartleLUT * const lut,STLQueuedFlag queued_flag,wxUint16 at_tick,wxTextCtrl *const status_text)
:STLCommandSetNoiseAmplitudeSPLXXX(dB,lut,queued_flag,at_tick),
STLClientCommandXXX(sock)
{
errorType rv=errNone;
	SetStatusText(status_text);
    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSIClientCommandSetNoiseAmplitudeSPLXXX ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

STLClientCommandSetNoiseAmplitudeSPLXXX::~STLClientCommandSetNoiseAmplitudeSPLXXX()
{

}


errorType STLClientCommandSetNoiseAmplitudeSPLXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");
//#warning ("remove following line. Need to use default 10 sec timeout")
	//GetSock().SetTimeout(1000);		//debugging
    //GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending STL SetNoiseAmplitudeSPLXXX record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the STLCommand id
    buffer=STL_COMMAND_SET_NOISE_AMPLITUDE_SPL;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------


//-----write the data, a float in this instance ----
	buff=GetAmplitudeSPL();
    GetSock().Write(&buff,sizeof(buff));  //write the frequency
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandSetNoiseAmplitudeSPL",errSocket);
    }

    return errNone;
}


//--- End SetNoiseAmplitudeSPLXXX ---------


/**************************************************************************************************
            class STLClientCommandOpenDeviceXXX
**************************************************************************************************/


/**************************************************************************************
	GSIClientCommandOpenDeviceXXX::STLClientCommandOpenDeviceXXX(wxSocketBase &sock, const wxString & device_str, wxTextCtrl  *status_text)
	ctor
***************************************************************************************/

STLClientCommandOpenDeviceXXX::STLClientCommandOpenDeviceXXX(wxSocketBase & sock,const wxString &dev_str,STLQueuedFlag queued_flag,STLAtTickParam at_tick,wxTextCtrl *const status_text)
:STLCommandOpenDeviceXXX(dev_str,queued_flag,at_tick),
STLClientCommandXXX(sock)
//:m_Sock(sock), m_StatusText(text)
{
errorType rv=errNone;
	SetStatusText(status_text);
    m_DeviceStr=dev_str;

    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandOpenDevice",errSocket);
        return;
    }
    int n=GetReturnNumInts();
	if(n==0)
	{
		//device failed to open
	}
    if(n>0)            //will return handle to this device and a node handle
    {
        wxInt16 *data;
        data=GetReturnData();
        m_hDevice=*data ;
    }
    if(n ==2)
    {   //can return less data on a failure

    }
	else	//device is open, Node failed to open
	{
		SetReturnCode(GSISocketReturnData::GSI_NOT_FOUND);
	}

    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}



STLClientCommandOpenDeviceXXX::~STLClientCommandOpenDeviceXXX()
{
}



/********************************************************************
    errorType STLClientCommandOpenDeviceXXX::Write(bool use_written_flag)
Used by a GSISocketClient to write to a GSISocketServer

Write:
WxInt16 Length of string
char[length of string]  (The device name)
********************************************************************/
errorType STLClientCommandOpenDeviceXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");


    GetSock().SetFlags(wxSOCKET_WAITALL);


    AppendText(_("\n-Sending OpenDevice record-\n"));
//-----------Write the socket command type id (A USER1 record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Write the GSICommand id
    buffer=STL_COMMAND_OPEN_DEVICE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);


	//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the Socket event handler

//below are read by the specific read function associated with this object

//----write the data length in bytes-----------------       //socket server will understand this format
    buffer=(wxInt16)m_DeviceStr.length();                   //length of data in bytes
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
        if(GetSock().Error())
            return(errFail);
//-----write the data sring in this instance ----
    GetSock().Write(m_DeviceStr.c_str(),m_DeviceStr.length());   //write the str
    if(GetSock().Error())
        return(errFail);
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandOpenDevice",errSocket);
    }

    return errNone;
}



//------ End STLClientCommandOpenDeviceXXX


/***********************************************************
class STLClientCommandCloseDeviceXXX :
	ctor
***********************************************************/
STLClientCommandCloseDeviceXXX::STLClientCommandCloseDeviceXXX(wxSocketBase & sock,wxInt16 h_dev,STLQueuedFlag queued_flag,STLAtTickParam at_tick,wxTextCtrl * const status_text)
:STLCommandCloseDeviceXXX(queued_flag,at_tick),
STLClientCommandXXX(sock)
{
errorType rv=errNone;
	SetStatusText(status_text);

	SetDeviceHandle(h_dev);

    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandCloseDevice",errSocket);
        return;
    }
    int n=GetReturnNumInts();
    if(n>0)            //will return handle to this device and a node handle
    {
        wxInt16 *data;
        data=GetReturnData();
    }

    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());

}

/*!
STLClientCommandCloseDeviceXXX::~STLClientCommandCloseDeviceXXX()
*/
STLClientCommandCloseDeviceXXX::~STLClientCommandCloseDeviceXXX()
{
	SetWrittenFlag(true);		//
}

void STLClientCommandCloseDeviceXXX::Init()
{

}

/*!
errorType STLClientCommandCloseDeviceXXX::Write(bool use_written_flag)
*/
errorType STLClientCommandCloseDeviceXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");


    GetSock().SetFlags(wxSOCKET_WAITALL);


    AppendText(_("\n-Sending CloseDevice record-\n"));
//-----------Write the socket command type id (A USER1 record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);

//Write the GSICommand id
    buffer=STL_COMMAND_CLOSE_DEVICE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
        return(errFail);


	//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the Socket event handler

//below are read by the specific read function associated with this object

//----write the data -----------------       //socket server will understand this format
    buffer=GetDeviceHandle();                   //length of data in bytes
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
        if(GetSock().Error())
            return(errFail);
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSICommandOpenDevice",errSocket);
    }

    return errNone;
}

//END  STLClientCommandCloseDevice::~STLClientCommandCloseDevice()


//------ End STLCLientCommandCloseDeviceXXX


/*!
******************************************************
STLClientCommandResetXXX::STLClientCommandResetXXX
	ctor
******************************************************/
STLClientCommandResetXXX::STLClientCommandResetXXX(
		wxSocketBase & sock,
		STLQueuedFlag queued_flag,
		wxUint16 at_tick,
		wxTextCtrl *const status_text)
:STLClientCommandXXX(sock)
{
errorType rv=errNone;
	SetCommandString("STLCientCommandResetXXX");
	SetQueuedFlag(queued_flag);
	SetAtTick(at_tick);
	SetStatusText(status_text);

	rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandReset ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

/*!
STLClientCommandResetXXX::~STLClientCommanResetXXX()
dtor
*/
STLClientCommandResetXXX::~STLClientCommandResetXXX()
{

}


/*!
errorType STLClientCommandReset::Write(bool use_written_flag)
*/
errorType STLClientCommandResetXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");

    AppendText(_("\n-Sending STL Reset record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_RESET;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------

//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandReset",errSocket);
    }

    return errNone;
}
//-----End STLClientCommandResetXXX----------------


/*!
************************************************************
STLClientCommandSetChannelXXX::STLClientCommandSetChannelXXX
	ctor
************************************************************/

STLClientCommandSetChannelXXX::STLClientCommandSetChannelXXX(wxSocketBase & sock,wxInt16 chan,STLQueuedFlag queued_flag,STLAtTickParam at_tick,wxTextCtrl *const status_text)
:STLCommandSetChannelXXX(chan, queued_flag,at_tick),STLClientCommandXXX(sock)
{
errorType rv=errNone;

SetStatusText(status_text);
	SetChannel(chan);

    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSIClientCommandSetChannelXXX ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

/*!
STLClientCommandSetChannelXXX::~STLClientCommandSetChannelXXX()
*/
STLClientCommandSetChannelXXX::~STLClientCommandSetChannelXXX()
{

}


errorType STLClientCommandSetChannelXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");

    AppendText(_("\n-Sending STL SetChannelXXX record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_SET_CHANNEL;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------


//-----write the data ----
	buff=GetChannel();
    GetSock().Write(&buff,sizeof(buff));  //write the frequency
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandSetChannel",errSocket);
    }

    return errNone;
}

//--- End STLClientCommandSetChannelXXX


/*!
******************************************************
STLClientCommandAcquireXXX::STLClientCommandAcquireXXX
	ctor
******************************************************/
STLClientCommandAcquireXXX::STLClientCommandAcquireXXX(wxSocketBase & sock,wxUint16 num_data_points,STLQueuedFlag queued_flag,wxUint16 at_tick,wxTextCtrl * const status_text)
:STLClientCommandXXX(sock)
{
errorType rv=errNone;
	SetStatusText(status_text);
	SetAtTick(at_tick);
	SetQueuedFlag(queued_flag);
	SetNumDataPoints(num_data_points);

    rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSIClientCommandAcquire ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
//ADC data is in buffer. Use GetReturnDataBuffer() to access

}

/*!
STLClientCommandAcquire::~STLClientCommandAcquire()
*/
STLClientCommandAcquireXXX::~STLClientCommandAcquireXXX()
{

}

/*!
errorType STLClientCommandAcquireXXX::Write(bool use_written_flag)
*/
errorType STLClientCommandAcquireXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");
//#warning ("remove following line. Need to use default 10 sec timeout")
	//GetSock().SetTimeout(1000);		//debugging
    //GetSock().SetFlags(wxSOCKET_WAITALL);

    AppendText(_("\n-Sending STL Acquire record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_ACQUIRE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------


//-----write the data ----
	buff=GetNumDataPoints();
    GetSock().Write(&buff,sizeof(buff));  //write the frequency
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandAcquire",errSocket);
    }

    return errNone;
}

//end class STLClientCommandAcquireXXX




/*!
******************************************************
STLCommandExecuteXXX::STLCommandExecuteXXX
	ctor
******************************************************/
STLClientCommandExecuteXXX::STLClientCommandExecuteXXX(wxSocketBase & sock,STLQueuedFlag queued_flag,wxUint16 at_tick,wxTextCtrl *const status_text)
:STLCommandExecuteXXX(queued_flag,at_tick),
STLClientCommandXXX(sock)
{
errorType rv=errNone;

	SetStatusText(status_text);

	rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandExecute ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

/*!
STLClientCommandExecuteXXX::~STLClientCommandExecuteXXX()
dtor
*/
STLClientCommandExecuteXXX::~STLClientCommandExecuteXXX()
{

}



/*!
errorType STLClientCommandExecuteXXX::Write(bool use_written_flag)
*/
errorType STLClientCommandExecuteXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");

    AppendText(_("\n-Sending STL Execute record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_EXECUTE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------

//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandExecute",errSocket);
    }

    return errNone;
}
//End class STLCommandExecuteXXX



/*!
******************************************************
STLCommandWaitForCompletionXXX::STLCommandWaitForCompletionXXX
	ctor
******************************************************/
STLClientCommandWaitForCompletionXXX::STLClientCommandWaitForCompletionXXX(wxSocketBase & sock,STLQueuedFlag queued_flag,wxUint16 at_tick,wxTextCtrl *const status_text)
:STLCommandWaitForCompletionXXX(queued_flag,at_tick),
STLClientCommandXXX(sock)

{
errorType rv=errNone;

	SetStatusText(status_text);
   rv=Write(true);                    //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandWaitForCompletion ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

/*!
STLClientCommandWaitForCompletionXXX::~STLClientCommandWaitForCompletionXXX()
dtor
*/
STLClientCommandWaitForCompletionXXX::~STLClientCommandWaitForCompletionXXX()
{

}



/*!
errorType STLClientCommandWaitForCompletionXXX::Write(bool use_written_flag)
*/
errorType STLClientCommandWaitForCompletionXXX::Write(bool use_written_flag)
{
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");

    AppendText(_("\n-Sending STL WaitForCompletion record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_WAIT_FOR_COMPLETION;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	wxUint16 buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------

//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLClientCommandReset",errSocket);
    }

    return errNone;
}
//-----------End STLClassWaitForCompletionXXX----------------



/*!
******************************************************
STLClientCommandSetAmplitudeLUTValueXXX::STLClientCommandSetSetAmplitudeLUTValueXXX
	ctor
******************************************************/
STLClientCommandSetAmplitudeLUTValueXXX::STLClientCommandSetAmplitudeLUTValueXXX(
						wxSocketBase & sock,
						wxInt16 index,
						wxInt16 value,
						wxTextCtrl *const status_text)
:STLCommandSetAmplitudeLUTValueXXX(index,value),
STLClientCommandXXX(sock)
{
errorType rv=errNone;
	//SetIndex(index);
	//SetValue(value);
	SetStatusText(status_text);
    rv=Write(true);                //internally written. Don't us m_written flag if calling Write() directly
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"GSIClientCommandSetChannel ctor",rv);
        return;
    }
    DisplayReturnCode(GetStatusText());
    DisplayData(GetStatusText());
}

/*!
STLClientCommandSetAmplitudeLUTValueXXX::~STLClientCommandSetAmplitudeLUTValueXXX()
*/
STLClientCommandSetAmplitudeLUTValueXXX::~STLClientCommandSetAmplitudeLUTValueXXX()
{

}



errorType STLClientCommandSetAmplitudeLUTValueXXX::Write(bool use_written_flag)
{
	wxInt16 buff;
    errorType rv=errNone;
    wxASSERT_MSG(!(use_written_flag && IsWritten()),"ctor has already called Write()!");

    AppendText(_("\n-Sending STL SetAmplitudeLUTValueXXX record-\n"));
//-----------Write the socket command type id (A STL socket record in this case) --------------
    wxInt16 buffer=(wxInt16)GSI_SOCKET_USER1;
    GetSock().Write(&buffer, sizeof (wxInt16)); //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}


//Write the STLCommand id
    buffer=STL_COMMAND_SET_AMPLITUDE_LUT_VALUE;                        //command id
    GetSock().Write(&buffer, sizeof (wxInt16));                    //write the id
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Write the queued flag
	buff=GetQueuedFlag();
    GetSock().Write(&buff,sizeof (buff));           //write the QUEUED/IMMEDIATE flag
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}
//Write the atTick variable
	buff=GetAtTick();
    GetSock().Write(&buff, sizeof buff);                    //write the atTick variable
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//Above are read by the STLServer event handler

//below are read by the specific read function associated with this object

//----------------------------------------------


//-----write the data ----
	buff=GetIndex();
    GetSock().Write(&buff,sizeof(buff));  //write the index
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//-----write the data ----
	buff=GetValue();
    GetSock().Write(&buff,sizeof(buff));  //write the value
    if(GetSock().Error())
	{
		MapWxSocketError sockErr(GetSock().LastError());
        return(sockErr.GetErrorType());
	}

//----------------------------------------------
    if(use_written_flag)        //ctor will set this flag, direct calls will not, thus can send multiple times
    {
        //m_written=true;
        SetWrittenFlag(true);
    }

    //-----record has been written, so read the data returned by the server indicating success/fail + any additional data-----
    rv=ReadServerReturnRecord();
    if(rv)
    {
        GSI_SOCKET_THROW(GetSock(),"STLCommandSetChannel",errSocket);
    }

    return errNone;
}
//End STLClientCommandSetAmplitudeLUTValueXXX()

#endif //#ifdef STARTLE
