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
#ifndef STL_CLIENT_COMMANDS_HPP_INCLUDED
#define STL_CLIENT_COMMANDS_HPP_INCLUDED

#include "..\socket\gsidefs.h"
#include "..\socket\STLSocketCommands.hpp"
#include "..\socket\GSIClientCommands.hpp"

/*!
class STLClientCommandXXX : public GSIClientCommandXXX
*/

class STLClientCommandXXX :public GSIClientCommandXXX
{
public:

    STLClientCommandXXX(wxSocketBase & sock);  //TextCtrl can be NULL so no reference
    virtual ~STLClientCommandXXX();
	void	Init();

private:

};

class STLSocketClientXXX //: public GSIClientCommandXXX
{
public:
	//STLClientCommand();

	STLSocketClientXXX(wxSocketBase & sock);
	virtual ~STLSocketClientXXX();

	void	Init();
//Access functions
	wxSocketBase       & GetSock()     const {return m_Sock;}

private:
	wxSocketBase &m_Sock;
};


/*!
class STLClientCommandSetFrequencyXXX : public STLClientCommandXXX
*/
class STLClientCommandSetFrequencyXXX : public STLCommandSetFrequencyXXX,public STLClientCommandXXX
{
public:
	STLClientCommandSetFrequencyXXX(wxSocketBase & sock,
		wxInt16 f,
		STLQueuedFlag queued_flag=STL_IMMEDIATE,
		STLAtTickParam at_tick=0,
		wxTextCtrl *const status_text=NULL);

	virtual ~STLClientCommandSetFrequencyXXX();

		errorType Write(bool use_written_flag);			//A socket write command

private:

};



/*!
class STLClientCommandSetAmplitudeSPLXXX : public STLClientCommandXXX
*/
class STLClientCommandSetAmplitudeSPLXXX : public STLCommandSetAmplitudeSPLXXX,public STLClientCommandXXX
{
public:
	STLClientCommandSetAmplitudeSPLXXX(wxSocketBase & sock,wxInt16 amplitude ,StartleLUT * const lut,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
	STLClientCommandSetAmplitudeSPLXXX(wxSocketBase & sock,wxInt16 amplitude ,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandSetAmplitudeSPLXXX();

			errorType Write(bool use_written_flag);			//A socket write command
private:
};


/*!
class STLClientCommandSetNoiseAmplitudeSPLXXX : public STLClientCommandXXX
*/
class STLClientCommandSetNoiseAmplitudeSPLXXX : public STLCommandSetNoiseAmplitudeSPLXXX ,public STLClientCommandXXX
{
public:
	STLClientCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,wxInt16 dB,StartleLUT * const lut,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
	STLClientCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,wxInt16 dB,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandSetNoiseAmplitudeSPLXXX();

	void	SetAmplitude(wxInt16 dB)	{m_dB=dB;}
	errorType Write(bool use_written_flag);

	wxInt16	 GetAmplitudeParam() const {return m_dB;}
private:
	wxInt16	m_dB;

};


/*!
class STLClientCommandOpenDeviceXXX
Derived from the underlying STLCommandOpenDevice
and with STLClientCommand adding Socket write and read of returned data
*/
class STLClientCommandOpenDeviceXXX : public STLCommandOpenDeviceXXX,public STLClientCommandXXX
{
public:

	STLClientCommandOpenDeviceXXX(wxSocketBase & sock,const wxString &dev_str="",STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandOpenDeviceXXX();
	void Init();
	errorType Write(bool use_written_flag);

//Access functions

	wxString GetDeviceString() const {return m_DeviceStr;}
	wxInt16	GetComPortNumber() const {return m_comPortNumber;}
	GSIDevHandle	GetDeviceHandle() const {return m_hDevice;}

private:
    wxString    m_DeviceStr;
	wxInt16		m_comPortNumber;				//parameter sent to this class (a wxInt16 comport number in this instance)
    GSIDevHandle    m_hDevice;               //handle to the device (returned from GSIServer)

	//These access functions are only used after a succesful parameter check
	void SetDeviceString(const wxString& device_str) {m_DeviceStr=device_str;}
	void SetComPortNumber(wxInt16 num){m_comPortNumber=num;}
	void SetDeviceHandle(GSIDevHandle h_device){m_hDevice=h_device;}

};


/*!
class STLClientCommandCloseDeviceXXX : public STLClientCommandXXX
*/
class STLClientCommandCloseDeviceXXX : public STLCommandCloseDeviceXXX,public STLClientCommandXXX
{
public:
	STLClientCommandCloseDeviceXXX(wxSocketBase & sock,wxInt16 h_dev,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandCloseDeviceXXX();
	void Init();
	errorType Write(bool use_written_flag);

//Access functions


private:


};


/*!
class STLClientCommandResetXXX : public STLClientCommandXXX
*/
class STLClientCommandResetXXX : public  STLCommandResetXXX,public STLClientCommandXXX
{
public:
	STLClientCommandResetXXX(wxSocketBase & sock,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);

    virtual ~STLClientCommandResetXXX();

	errorType Write(bool use_written_flag);

private:

};


/*!
class STLClientCommandSetChannelXXX : public STLClientCommandXXX
*/
class STLClientCommandSetChannelXXX : public STLCommandSetChannelXXX,public STLClientCommandXXX
{
public:
	STLClientCommandSetChannelXXX(wxSocketBase & sock,wxInt16 chan,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandSetChannelXXX();

	//void		SetChannel(wxInt16 chan)	{m_chan=chan;}
	errorType Write(bool use_written_flag);

	//wxInt16 GetChannel() const {return m_chan;}
private:
	//wxInt16		m_chan;

};



/*!
class STLClientCommandAcquireXXX : public STLClientCommandXXX
*/
class STLClientCommandAcquireXXX: public STLCommandAcquireXXX, public STLClientCommandXXX
{
public:

	STLClientCommandAcquireXXX(wxSocketBase & sock,wxUint16 num_data_points,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl * const status_text=NULL);
    virtual ~STLClientCommandAcquireXXX();
	errorType Write(bool use_written_flag);

private:

};



/*!
class STLClientCommandExecuteXXX : public STLClientCommandXXX
*/
class STLClientCommandExecuteXXX : public STLCommandExecuteXXX,public STLClientCommandXXX
{
public:

	STLClientCommandExecuteXXX(wxSocketBase & sock,
							STLQueuedFlag queued_flag=STL_IMMEDIATE,
							STLAtTickParam at_tick=0,
							wxTextCtrl * const status_text=NULL);
    virtual ~STLClientCommandExecuteXXX();

	errorType Write(bool use_written_flag);

private:

};



/*!
class STLClientCommandWaitForCompletionXXX : public STLClientCommandXXX
*/
class STLClientCommandWaitForCompletionXXX : public STLCommandWaitForCompletionXXX, public STLClientCommandXXX
{
public:

	STLClientCommandWaitForCompletionXXX(wxSocketBase & sock,STLQueuedFlag queued_flag=STL_IMMEDIATE,STLAtTickParam at_tick=0,wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandWaitForCompletionXXX();
	errorType Write(bool use_written_flag);

//Access functions


private:

};


/*!
class STLClientCommandSetAmplitudeLUTValueXXX : public STLClientCommandXXX
*/
class STLClientCommandSetAmplitudeLUTValueXXX : public STLCommandSetAmplitudeLUTValueXXX, public STLClientCommandXXX
{
public:
	STLClientCommandSetAmplitudeLUTValueXXX(
		wxSocketBase & sock,
		wxInt16 index,
		wxInt16 value,
		wxTextCtrl *const status_text=NULL);
    virtual ~STLClientCommandSetAmplitudeLUTValueXXX();

	errorType Write(bool use_written_flag);

private:
};

#endif
