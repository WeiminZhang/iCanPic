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
#ifndef STARTLE_FRAME_HPP_INCLUDED
#define STARTLE_FRAME_HPP_INCLUDED

#include "gsi.hpp"
//forward references
#include "stlcommands.hpp"
#include "..\socket\STLServer.hpp"
#include "DlgLUT.h"

class STLServerCommandXXX;

class StartleFrame : public GSIFrame
{
public:
    StartleFrame();
    virtual ~StartleFrame();
	void	Init();

	void OnStartleTest1(wxCommandEvent& event);
	void OnStartleTest2(wxCommandEvent& event);
	//LUT helper dialog events
	void OnLUTHelper(wxCommandEvent& event);
	//Next two are not really event handlers, they are called by
	//an event handler
	void		OnDlgLUTToneButton(wxCommandEvent& event,DlgLUT *lut);
	void		OnDlgLUTButton(wxCommandEvent& event, DlgLUT *Dlg);
	errorType	ReadLUT(wxTextFile * const f, int *id);

	void OnDlgLUTToneButtonX();
	//Load LUT dlg
	void OnLUTLoad(wxCommandEvent& event);

	errorType	ProcessServerCommand(STLCommandXXX &Cmd,wxSocketBase &sock);

	errorType	ResetXXX(STLCommandResetXXX & cmd);

	errorType	SetAmplitudeSPLXXX(STLCommandSetAmplitudeSPLXXX & cmd);

	errorType	SetNoiseAmplitudeSPLXXX(STLCommandSetNoiseAmplitudeSPLXXX & cmd);

	errorType	SetFrequencyXXX(STLCommandSetFrequencyXXX & cmd);

	errorType	AcquireXXX(STLCommandAcquireXXX & cmd);

	errorType	SetChannelXXX(STLCommandSetChannelXXX & cmd);

	errorType	ExecuteXXX(STLCommandExecuteXXX & cmd);

	errorType	WaitForCompletionXXX(STLCommandWaitForCompletionXXX & cmd);

	errorType	GetStatusXXX(STLCommandGetStatusXXX & cmd);

	errorType	SetAmplitudeLUTValueXXX(STLCommandSetAmplitudeLUTValueXXX &cmd);
	//errorType	SetNoiseAmplitudeLUTValueXXX(STLCommandSetNoiseAmplitudeLUTValueXXX &cmd);


	errorType	WriteADCDataToDisk(const wxString & fname,wxInt16 num_ints,wxInt16 *const buffer);

//following are called from the SockCommands or directly by the StartleServer
//or directly by STLFrame
	//errorType	CommandReset(bool flag=STL_IMMEDIATE);
	void			ADCSwapByteOrder(); //Swap ADC array byte order
//Access functions
	wxInt16 * const		GetADCBuffer() const {return m_buff;}
	//const GSIDevHandle	GetDevHandle() const {return m_hDev;}
	//void				SetDevHandle(GSIDevHandle hdev) {m_hDev=hdev;}
	const GSINodeHandle	GetNodeHandle() const {return m_hNode;}
	StartleServer & GetStartleServer() {return (StartleServer &) GetGSIServer();}
#if GSI_USE_SHARED_POINTERS
			//boost::shared_ptr<StartleLUT> GetAmplitudeLUT()  {return GetStartleServer().GetAmplitudeLUT();}
			boost::shared_ptr<StartleLUT> GetAmplitudeLUT();//  {return GetStartleServer().GetAmplitudeLUT();}
			boost::shared_ptr<StartleLUT>GetNoiseAmplitudeLUT() {return GetStartleServer().GetNoiseAmplitudeLUT();}

#else
			//StartleLUT *const			GetAmplitudeLUT()  {return GetStartleServer().GetAmplitudeLUT();}
			//StartleLUT *const			GetNoiseAmplitudeLUT() {return GetStartleServer().GetNoiseAmplitudeLUT();}
#endif

			wxUint16				GetNumADCDataPoints() const {return m_numADCDataPoints;}
			errorType				SetNumADCDataPoints(wxUint16 num_data_points);
			bool					IsQueuedADC() const {return m_queuedADC;}
			void					SetQueuedADC(bool q_flag) {m_queuedADC=q_flag;}
			int						GetADCNumRead() const	{return m_numRead;}
			void					SetADCNumRead(int num_read) {m_numRead=num_read/sizeof(wxInt16);}

private:
			errorType		LoadTextFile(const wxString &fname,const wxTextFile & fp);
	wxMenu	*m_menuSTL;
	wxString m_DeviceString;
	//GSIDevHandle	m_hDev;
	GSINodeHandle	m_hNode;

//m_numADCDataPoints and m_queuedADC must both be in the StartleFrame
//because a socket client can send a queued ADC request which will not
//be acted upon till the corresponding Execute is found. The Acquire object
//will have been destroyed.
//Note that a RESET must set these back to zero

    int		m_numADCDataPoints;
	int		m_numRead;
    bool	m_queuedADC;
	wxInt16 * m_buff;
	DECLARE_EVENT_TABLE()
};



#endif  //STARTLE_FRAME_HPP_INCLUDED
