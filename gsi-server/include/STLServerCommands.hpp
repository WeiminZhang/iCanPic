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
#ifndef STL_SERVER_COMMANDS_HPP_INCLUDED
#define STL_SERVER_COMMANDS_HPP_INCLUDED

#include "STLSocketCommands.hpp"



/****************************************************************************
	Server commands read from the client and return ServerReturnRecord
Each STL command has a client and server class. Although it is not very elegant
it helps to avoid confusion
*****************************************************************************/
//STLServerCommand
class STLServerCommandXXX //: public STLCommandXXX
{
public:
	STLServerCommandXXX(wxSocketBase & sock);

	virtual ~STLServerCommandXXX();
	void	Init();
	wxSocketBase       & GetSock()     const {return m_Sock;}
	virtual errorType Read();
	errorType ReadFixedFields(StartleRecord &r);	//read qflag, at_tick
//Access functions
    void			SetReadFlag(bool read=true)	{m_isRead=read;}
    const bool		IsRead ()   const			{return m_isRead;}


private:
	bool m_isRead;
	wxSocketBase &m_Sock;
};



/**

These commands all use STLServerCommand as a base class

*/


/*!
class STLServerCommandSetFrequencyXXX : public STLServerCommandXXX
*/

class STLServerCommandSetFrequencyXXX : public STLCommandSetFrequencyXXX,GSISocketXXX
{
public:
    STLServerCommandSetFrequencyXXX(wxSocketBase & sock);
    virtual ~STLServerCommandSetFrequencyXXX();
    virtual errorType Read(wxSocketBase & sock);
	errorType	Execute(StartleFrame &Frame,GSISocketReturnData &sr);
private:
};



/*!
class STLServerCommandSetAmplitudeSPLXXX
*/
class STLServerCommandSetAmplitudeSPLXXX : public STLCommandSetAmplitudeSPLXXX,GSISocketXXX
{
public:
    
#if GSI_USE_SHARED_POINTERS
	STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock,boost::shared_ptr<StartleLUT> lut);
#else
	STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock,StartleLUT * const lut);
#endif
	STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock);		//No LUT version
    virtual ~STLServerCommandSetAmplitudeSPLXXX();
    virtual errorType Read(wxSocketBase &sock);
	errorType	Execute(StartleFrame &Frame,GSISocketReturnData &sr);
//Access functions

private:

};


/*!
class STLServerCommandSetAmplitudeXXX :public STLServerCommandXXX
*/
class STLServerCommandSetAmplitudeXXX :public STLServerCommandXXX
{
public:
		STLServerCommandSetAmplitudeXXX(wxSocketBase & sock);
		virtual ~STLServerCommandSetAmplitudeXXX();

private:
};




#if 0
/*!
class STLServerCommandSetAmplitudeSPLXXX
*/
class STLServerCommandSetAmplitudeSPLXXX : public STLCommandSetAmplitudeSPLXXX,GSISocketXXX
{
public:
    
#if GSI_USE_SHARED_POINTERS
	STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock,boost::shared_ptr<StartleLUT> lut);
#else
	STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock,StartleLUT * const lut);
#endif
	STLServerCommandSetAmplitudeSPLXXX(wxSocketBase & sock);		//No LUT version
    virtual ~STLServerCommandSetAmplitudeSPLXXX();
    virtual errorType Read(wxSocketBase &sock);
	errorType	Execute(StartleFrame &Frame,GSISocketReturnData &sr);
//Access functions

private:

};
#endif

/*!
class STLServerCommandSetNoiseAmplitudeSPLXXX
*/
class STLServerCommandSetNoiseAmplitudeSPLXXX : public STLCommandSetNoiseAmplitudeSPLXXX,GSISocketXXX
{
public:
    //STLServerCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,StartleLUT * const lut);
#if GSI_USE_SHARED_POINTERS
	STLServerCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,boost::shared_ptr<StartleLUT> lut);
#else
	STLServerCommandSetNoiseAmplitudeSPLXXX(wxSocketBase & sock,StartleLUT * const lut);
#endif

    virtual ~STLServerCommandSetNoiseAmplitudeSPLXXX();
    virtual errorType Read(wxSocketBase &sock);
	errorType	Execute(StartleFrame &Frame,GSISocketReturnData &sr);
//Access functions

private:

};





/*!
class STLServerCommandSetNoiseAmplitudeXXX :public STLServerCommand
*/
class STLServerCommandSetNoiseAmplitudeXXX :public STLServerCommandXXX
{
public:
		STLServerCommandSetNoiseAmplitudeXXX(wxSocketBase & sock);
		virtual ~STLServerCommandSetNoiseAmplitudeXXX();
private:
};




/*!
class STLServerCommandSetDelayXXX :public STLServerCommandXXX
*/
class STLServerCommandSetDelayXXX :public STLServerCommandXXX
{
public:
		STLServerCommandSetDelayXXX(wxSocketBase & sock);
		virtual ~STLServerCommandSetDelayXXX();
private:
};


/*!
class STLServerCommandAcquireXXX :public STLServerCommandXXX
*/

class STLServerCommandAcquireXXX:public STLCommandAcquireXXX,GSISocketXXX
{
public:
		STLServerCommandAcquireXXX(wxSocketBase & sock);
		virtual ~STLServerCommandAcquireXXX();
		virtual errorType Read(wxSocketBase &sock);
		errorType Execute(StartleFrame &Frame,GSISocketReturnData &sr);
private:
};



/*!
class STLServerCommandSetChannelXXX
Server will receive channel information from client. It will then
fill the GSIRecord and send the record to the correct node
*/
class STLServerCommandSetChannelXXX : public STLCommandSetChannelXXX,GSISocketXXX
{
public:
    STLServerCommandSetChannelXXX(wxSocketBase & sock);
    virtual ~STLServerCommandSetChannelXXX();
	virtual errorType Read(wxSocketBase &sock);
	errorType Execute(StartleFrame &Frame,GSISocketReturnData &sr);

//Access functions
private:
};



/*!
class STLServerCommandExecuteXXX :public STLServerCommandXXX
*/
class STLServerCommandExecuteXXX :public STLCommandExecuteXXX,GSISocketXXX
{
public:
		STLServerCommandExecuteXXX(wxSocketBase & sock);
		virtual	~STLServerCommandExecuteXXX();

		virtual errorType Read(wxSocketBase &sock);
		errorType Execute(StartleFrame &Frame,GSISocketReturnData &sr);
private:

};


/*!
class STLServerCommandToneXXX :public STLServerCommandXXX
*/

class STLServerCommandToneXXX :public STLServerCommandXXX
{
public:
		STLServerCommandToneXXX(wxSocketBase & sock);
		virtual ~STLServerCommandToneXXX();
private:
};


/*!
class STLServerCommandGetStatusXXX :public STLServerCommandXXX
*/
class STLServerCommandGetStatusXXX :public STLServerCommandXXX
{
public:
		STLServerCommandGetStatusXXX(wxSocketBase & sock);
		virtual ~STLServerCommandGetStatusXXX();
		virtual errorType Read();        //actually, nothing to read in this implementation
private:
};



/*!
class STLServerCommandWaitForCompletionXXX :public STLServerCommandXXX
*/
class STLServerCommandWaitForCompletionXXX :public STLCommandWaitForCompletionXXX,GSISocketXXX
{
public:
		STLServerCommandWaitForCompletionXXX(wxSocketBase & sock);
		virtual ~STLServerCommandWaitForCompletionXXX();
		virtual errorType Read(wxSocketBase &sock);
		errorType Execute(StartleFrame &Frame,GSISocketReturnData &sr);

private:
};



/*!
class STLServerCommandSetAmplitudeLUTValueXXX :public STLServerCommandXXX
*/
class STLServerCommandSetAmplitudeLUTValueXXX :public STLCommandSetAmplitudeLUTValueXXX,GSISocketXXX
{
public:
		STLServerCommandSetAmplitudeLUTValueXXX(wxSocketBase & sock);
		virtual ~STLServerCommandSetAmplitudeLUTValueXXX();

	    virtual errorType Read(wxSocketBase &sock);
		errorType Execute(StartleFrame &Frame,GSISocketReturnData &sr);
private:
};


/*!
class STLServerCommandSetNoiseAmplitudeLUTValueXXX :public STLServerCommandXXX
*/
class STLServerCommandSetNoiseAmplitudeLUTValueXXX :public STLCommandSetNoiseAmplitudeLUTValueXXX,GSISocketXXX
{
public:
		STLServerCommandSetNoiseAmplitudeLUTValueXXX(wxSocketBase & sock);
		virtual ~STLServerCommandSetNoiseAmplitudeLUTValueXXX();

	    virtual errorType Read(wxSocketBase &sock);
		errorType Execute(StartleFrame &Frame,GSISocketReturnData &sr);
private:
};





/*!
STLServerCommandOpenDeviceXXX:public STLServerCommandXXX
*/
class STLServerCommandOpenDeviceXXX: public STLCommandOpenDeviceXXX,GSISocketXXX
{
public:
		STLServerCommandOpenDeviceXXX(wxSocketBase & sock);
	    virtual ~STLServerCommandOpenDeviceXXX();
		//errorType Read();
		GSIDevHandle	GetDeviceHandle(){return m_hDev;}
		wxString		GetDeviceString() {return m_DeviceStr;}
		errorType		Execute(class StartleFrame &Frame,GSISocketReturnData &sr);	//execute the class specific command
		errorType		Read(wxSocketBase &sock);
protected:
	void			SetDeviceHandle(GSIDevHandle h_dev) {m_hDev=h_dev;}
private:
		GSIDevHandle	m_hDev;
		wxString		m_DeviceStr;
};



/*!
class STLServerCommandCloseDeviceXXX:public STLServerCommandXXX
*/
class STLServerCommandCloseDeviceXXX:public STLCommandCloseDeviceXXX,GSISocketXXX
{
public:
			STLServerCommandCloseDeviceXXX(wxSocketBase & sock); //server side ctor
    virtual	~STLServerCommandCloseDeviceXXX();

	errorType		Read(wxSocketBase &sock);
	errorType		Execute(StartleFrame &Frame,GSISocketReturnData &sr);
private:
};



/*!
class STLServerCommandResetXXX
*/
class STLServerCommandResetXXX : public STLCommandResetXXX,GSISocketXXX
{
public:
		STLServerCommandResetXXX(wxSocketBase & sock);
		virtual ~STLServerCommandResetXXX();
	    virtual errorType Read(wxSocketBase & sock);
		errorType	Execute(StartleFrame &Frame,GSISocketReturnData &sr);

//Access functions
private:
};


#endif
