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
#ifndef STL_SOCKET_COMMANDS_HPP_INCLUDED
#define STL_SOCKET_COMMANDS_HPP_INCLUDED


#include <gerror.h>
#include "gsidefs.h"
#include "STLCommands.hpp"
#include "GSISocketCommands.hpp"


//Use typedefs for command parameter sent through the socket
//This ensures Client and Server send/receive correct data size
//Have to do this as the base class for the two is not the same.
//parameters could be declared differently otherwise
typedef wxUint16 STLFrequencyParam;			//In Hz
typedef wxUint16 STLAmplitudeSPLParam;		//in dB
typedef wxUint16 STLNoiseAmplitudeSPLParam;	//in dB
typedef wxUint16 STLAtTickParam;			//in msec
typedef wxInt16 STLOpenDeviceParam;		//comport number in this instance
typedef GSIDevHandle STLCloseDeviceParam;	//GSIDevHandle
typedef wxUint16 STLAmplitudeParam;			//in binary
typedef wxUint16 STLNoiseAmplitudeParam;	//in binary
typedef wxInt16 STLChannelParam;			//

//forward references
class wxSocketBase;

const size_t STLCommandArrayMaxElements=GSIRecordMaxLength+1;        //+1 for the inital flag


/*********************************************************
	class STLSocketCommandXXX :public  GSISocketCommandXXX
	base class for all startle commands
**********************************************************/
//Need to add a StartleRecord to all the classes derived from here
class STLSocketCommandXXX :public  GSISocketCommandXXX
{
public:
	STLSocketCommandXXX(wxSocketBase & sock,GSICommandXXX &STLCmd,wxTextCtrl * const status_text=NULL);
	explicit STLSocketCommandXXX();
    virtual ~STLSocketCommandXXX();

	void		Init();

	bool		IsFilled() {return m_filledFlag;}
	void		SetFilledFlag(bool flag) {m_filledFlag=flag;}

//Access functions

	void		SetQueuedFlag(STLQueuedFlag queued_flag) {m_queuedFlag=queued_flag;}
	void		SetAtTick(wxUint16 at_tick)			{m_atTick=at_tick;}
	void		SetNode(wxUint16 node=GSI_NODE_THIS){m_node=node;}
	void		SetCommand(enum STLEnumCommand command)		{m_command=command;}

	STLQueuedFlag	const GetQueuedFlag() const {return m_queuedFlag;}
	wxUint16	const GetAtTick() const {return m_atTick;}
	wxUint16	const GetNode() const {return m_node;}
	STLEnumCommand const GetCommand() const {return m_command;}

	wxInt16		GetLastError(){return m_lastError;}
	void		SetLastError(errorType e){m_lastError=e;}

private:


	STLEnumCommand m_command;
	wxInt16	m_node;

	STLQueuedFlag m_queuedFlag;
	wxUint16 m_atTick;
	bool	m_filledFlag;
	errorType	m_lastError;
	bool	m_reportStatusFlag;
};


#if 0
class STLSocketReturnData :public GSISocketReturnData
{
public:
enum GSISocketReturnValue		//tack on our values at the end of the enum
{
	STL_BAD_FREQUENCY=GSI_FIRST_USER_GSI_SOCKET_VALUE+1,
	STL_BAD_AMPLITUDE,
	STL_BAD_PRESCALER
};
//GSISocketReturnData(wxSocketBase &sock,bool internal_flag=false);
//GSISocketReturnData(wxSocketBase &sock,enum GSISocketReturnValue sr,bool flag=false);
	STLSocketReturnData(wxSocketBase &sock,bool internal_flag=false);
    STLSocketReturnData(wxSocketBase &sock,enum STLSocketReturnValue sr,bool internal_flag=false);
	STLSocketReturnData(wxSocketBase &sock,int sr,bool internal_flag=false);
	//STLSocketReturnData(wxSocketBase &sock,enum GSISocketReturnValue sr,bool flag=false);
	//STLSocketReturnData(wxSocketBase &sock,enum STLSocketReturnValue sr,bool flag=false);
	virtual ~STLSocketReturnData();
private:

};
#endif

#endif
