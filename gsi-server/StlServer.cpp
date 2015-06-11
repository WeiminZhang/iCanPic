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
#  pragma implementation "STLServer.cpp"
#  pragma interface "STLServer.cpp"
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
//#include <wx/choicdlg.h>

#include "include/gsi.hpp"

#ifdef STARTLE_SERVER

#include "include/record.hpp"
#include "include/server.hpp"
#include "include/socket.hpp"
#include "startle.hpp"
#include "include/STLServerCommands.hpp"
#include "include/startleFrame.hpp"
#include "include/STLServer.hpp"



#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif

// --------------------------------------------------------------------------
// resources
// --------------------------------------------------------------------------


/********************************************************************************************************
        ctor
StartleServer::StartleServer(unsigned short sock_address,wxStatusBar *status_bar,wxTextCtrl *text_ctrl)


LM1972 uses .5dB steps up to 48dB attenuation, then 1dB steps up to 100dB
A value of 127=full attenuation (104dB)


0,1,2,3,4,5,6,7,8,9					//0-
10,11,12,13,14,15,16,17,18,19,
20,21,22,23,24,25,26,27,28,29,
30,31,32,33,34,35,36,37,38,39,
40,41,42,43,44,45,46,47,48,49,
50,51,52,53,54,55,56,57,58,59,
60,61,62,63,64,65,66,67,68,69,
70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,
90,91,92,93,94,95,96,97,98,99,
100,101,102,103,104,105,106,107,108,109,
110,111,112,113,114,115,116,117,118,119,
120,121,122,123,124,125,126,127				//

M1972
  MSB: LSB
Address Register (Byte 0)
0000 0000 Channel 1
0000 0001 Channel 2
0000 0010 Channel 3
Data Register (Byte 1)
Contents Attenuation Level dB
0000 0000 0.0
0000 0001 0.5
0000 0010 1.0
0000 0011 1.5
: : : : : : :
0001 1110 15.0
0001 1111 15.5
0010 0000 16.0
0010 0001 16.5
0010 0010 17.0
: : : : : : :
0101 1110 47.0
0101 1111 47.5
0110 0000 48.0
0110 0001 49.0
0110 0010 50.0
: : : : : : :
0111 1100 76.0
0111 1101 77.0
0111 1110 78.0
0111 1111 100.0 (Mute)
1000 0000 100.0 (Mute)
: : : : : : :

*********************************************************************************************************/

StartleServer::StartleServer(unsigned short sock_address,wxStatusBar *status_bar,wxTextCtrl *text_ctrl,StartleFrame *startle_frame)
:GSIServer(sock_address,status_bar,text_ctrl,startle_frame) //send -1 as GSISockServer so that ctor doesn't construct it
{
//init data for the Amplitude LUT, 127 possible values, 1-125 dB and max output (127)
//0=MUTE (104dB output diconnected from input in LM1972)

    int dbLUT[STLMaxDB+1]={
		      127,												 //100dB
				  126, 125, 124, 123, 122, 121, 120, 119, 118,   //70-78dB attenuation LM1972 .5dB steps low-> high volume
              117, 116, 115, 114, 113, 112, 111, 110, 109, 108,   //60-69dB attenuation
              107, 106, 105, 104, 103, 102, 101, 100, 99,  98,	  //50-59dB attenuation
              97,  96,											  //48-49dB attenuation
						95,  94,  93,  92,  91,  90,  89,  88,	  //44-47.5dB
              87,  86,  85,  84,  83,  82,  81,  80,  79,  78,	  //39-43.5dB attenuation
              77,  76,  75,	 74,  73,  72,  71,  70,  69,  68,	  //34-38.5dB attenuation										  //50-48dB attenuation
			  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,	  //29-33.5dB attenuation
			  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,	  //24-28.5dB attenuation
			  47,  46,  45,  44,  43,  42,  41,  40,  39,  38,	  //19-23.5dB attenuation
			  37,  36,  35,  34,  33,  32,  31,  30,  29,  28,	  //14-18.5dB attenuation
			  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,	  //9-13.5dB attenuation
			  17,  16,  15,  14,  13,  12,  11,  10,  9,   8,	  //4-8.5dB attenuation
			  7,   6,   5,   4,   3,   2,   1,   0 				  //0.0-3.5dB
              };


    m_StartleFrame=startle_frame;
    m_queuedADC=false;           //are we awaiting an ADC? ie will startle box send us data
    m_numADCDataPoints=0;

    m_dBLUT=DEBUG_NEW LUT(STLMaxLUTIndex+1,STLMaxAmplitudeDB,dbLUT);
    m_dBNoiseLUT=DEBUG_NEW LUT(STLMaxLUTIndex+1,STLMaxNoiseAmplitudeDB,dbLUT);
}

/********************************************************************************************
    dtor
StartleServer::~StartleServer()
*********************************************************************************************/
StartleServer::~StartleServer()
{
    errorType rv;
	rv=errNone;
	//STLCommandReset cmd();
    //rv=Startle()->SockCommandReset(cmd);         //turn off the box as we quit
	//rv=Startle()->CommandReset(cmd);			//defaultsto immediate in any case
    delete m_dBLUT;
    delete m_dBNoiseLUT;
}





/*
	errorType StartleFrame::ProcessUser1Record(wxSocketBase &sock)

  This code runs on the server

Startle commands use an id of GSISocketUser1 (see enum in socket.hpp)
A GSI command uses GSISocketRecord. The server will call appropriate handler

Startle records are currently 4x6 bit words

This function is the socket server despathcer.
A socket record with an id of GSISocketUser1 has been sent by a socket client
The GSIServer::OnSocketEvent() function has captured the event and has called
this function
We must read the rest of the record, despatch the command to the device on the
node and send back success/fail record to the client.
The client is waiting and will timeout after 10 secs.

Client sends:
# GSI_SOCKET_USER1
# STL_COMMAND_SET_FREQUENCY
# queued flag
# atTick
//Above are always sent. Record specific data (if any) follows
# for example m_frequency, if this is a StartleCommandSetFreqency()

*/

errorType StartleServer::ProcessUser1Record(wxSocketBase &sock) //override this function only
{
errorType rv=errNone;
wxString s;
//StartleRecord stl;
wxInt16 recType;
wxInt16 queuedFlag;
wxUint16 atTick;

// Can't use ReadMsg as this is a wx extension
//have already read the GSISocketUser1 id, which is why we are here
//1)Command
//2)flag

    TextCtrl()->AppendText("\n\n--------- New startle record ----------\n");


    //sock.SetFlags(wxSOCKET_WAITALL);
/*--------------Read the STL Type field----------------*/
    sock.Read(&recType,sizeof (recType));
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
        return(sockErr.GetErrorCode());
	}

	s.Printf("Type=%i\n",recType);
    TextCtrl()->AppendText(s);

#if STL_DEBUG_USING_XXX //xxx functions read their own fixed length data
queuedFlag;
atTick;
#else

/*----------Read the STL QUEUED/IMMEDIATE Flag field--------------------*/
    sock.Read(&queuedFlag,sizeof (queuedFlag));
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
        return(sockErr.GetErrorCode());
	}

	s.Printf("flag=%i: ",queuedFlag);
    TextCtrl()->AppendText(s);

    if( (queuedFlag != STL_QUEUED) && (queuedFlag != STL_IMMEDIATE) )
    {
        TextCtrl()->AppendText("\n--- Flag error! ---\n");
        rv=errParameter;
    }
	else
	{
		if(queuedFlag==	STL_QUEUED)
			TextCtrl()->AppendText("Queued\n");
		else
			TextCtrl()->AppendText("Immediate\n");
	}
/*-----------------------------------------------------*/

/*--------------Read the atTick field----------------*/
    sock.Read(&atTick,sizeof(atTick));
    if(sock.Error())
	{
		MapWxSocketError sockErr(sock.LastError());
        return(sockErr.GetErrorCode());
	}

	s.Printf("at tick=%i \n",atTick);
    TextCtrl()->AppendText(s);
//----Read all fixed record elements---------------------------------------------

    TextCtrl()->AppendText("\n---------- Processing StartleRecord ----------\n");

    if(recType !=STL_COMMAND_OPEN_DEVICE && ! IsOpen())
    {
        //device isn't open and we are trying to send a command
        //return an error to the client
        //STLSocketReturnData sr(GSIClient(),(enum GSISocketReturnValue) GSINoNode);      //instantiate

        GSISocketReturnData sr(sock,GSISocketReturnData::GSI_NO_DEVICE);      //instantiate
        s.Printf("\n------------------- Failed: %s -----------------\n\n",sr.GetErrorString());
        TextCtrl()->AppendText(s);
        return errFail;
    }
#endif

#warning ("Need to test sending commands with no Device Opened")
	try
	{

	/*------------perform any Type specific transforms, or operations ------*/
		switch (recType)
		{
//STL_COMMAND_RESET and STL_COMMAND_SET_AMPLITUDE use the current
//method of communication. Work is done by StartleFrame::xxx functions
		case STL_COMMAND_RESET:
			{
				STLServerCommandResetXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;
		case STL_COMMAND_SET_AMPLITUDE_SPL:        //requires 1023=0, 0=1023.
			{
				STLServerCommandSetAmplitudeSPLXXX Cmd(sock,GetAmplitudeLUT());
				Cmd.SetStatusText(TextCtrl());

				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;
//New commands tested through the socket up to here
		case STL_COMMAND_SET_FREQUENCY:
			{
				STLServerCommandSetFrequencyXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;

			//rv=GetStartleFrame()->SetFrequency(sock,queuedFlag,atTick); //Hz is coming through the socket
			break;
		case STL_COMMAND_ACQUIRE:
			{
				STLServerCommandAcquireXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;

		case STL_COMMAND_WAIT_FOR_COMPLETION:
			{
				STLServerCommandWaitForCompletionXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;
		case STL_COMMAND_SET_CHANNEL:
			{
				STLServerCommandSetChannelXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());

				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;

//New commands implemented up to here
		case STL_COMMAND_SET_NOISE_AMPLITUDE_SPL:
			{
				STLServerCommandSetNoiseAmplitudeSPLXXX Cmd(sock,GetNoiseAmplitudeLUT());
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;
		case STL_COMMAND_EXECUTE:
			{
				STLServerCommandExecuteXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;

		case STL_COMMAND_OPEN_DEVICE:  //param0=comport number 1-6
			{
				STLServerCommandOpenDeviceXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				//rv=GetGSIFrame().ProcessServerCommand(Cmd);
#warning ("Need to update state of GSIFrame/StartleFrame with device information")

#if STL_DEBUG_USING_XXX
				//will stop ProcessServerCommandRead()
				//from reading fixed length records
				//so must add them here. Though not reqd for Open()
#endif
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;
			//rv=GetStartleFrame()->OpenDevice(sock); //inherited from GSIFrame
			break;
		case STL_COMMAND_CLOSE_DEVICE:  //param0=comport number 1-6
			{
				STLServerCommandCloseDeviceXXX Cmd(sock);
				Cmd.SetStatusText(TextCtrl());
				rv=GetStartleFrameRef().ProcessServerCommand(Cmd,sock);
				if(rv)
					return rv;
			}
			break;

			//rv=GetStartleFrame()->CloseDevice(sock);	//inherited from GSIFrame
			break;
		case STL_COMMAND_SET_AMPLITUDE_LUT_VALUE:           //set an element of the LUT p0=index, p1=value
			//rv=GetStartleFrame()->SetAmplitudeLUTValue(sock);
			break;
		case STL_COMMAND_SET_NOISE_AMPLITUDE_LUT_VALUE:           //set an element of the LUT p0=index, p1=value
			//rv=GetStartleFrame()->SetNoiseAmplitudeLUTValue(sock);
			break;
		default:
			//only instantiate the STLSocketReturnData here as it will write the code in it's dtor
			//STLSocketReturnData sr(*sock,GSISocketReturnData:: GSI_PARAM_ERROR,false);      //instantiate
			GSISocketReturnData sr(sock,GSISocketReturnData:: GSI_PARAM_ERROR,false);      //instantiate
			TextCtrl()->AppendText("\n-------- Bad Command ---------\n");
			rv=errParameter;        //this needs to return a SocketReturn
			break;
		}
	/*---------------------------------------------------------*/
		//TextCtrl()->AppendText(s);
		TextCtrl()->AppendText("\n--------Finished processing record---------\n");
		TextCtrl()->AppendText("\n-------------------------------------------\n");
	}
	catch(gsException &exc)
	{
        wxString s;
        s.Printf("Exception: %s. \nError code=%i %s",exc.GetUserString(),exc.GetErrorCode(),exc.GetString());
        TextCtrl()->AppendText(s);
		return exc.GetErrorCode();
	}
    return rv;
}


#endif //#ifdef STARTLE_SERVER
